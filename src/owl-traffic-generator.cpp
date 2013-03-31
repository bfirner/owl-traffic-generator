/*
 * Copyright (c) 2013 Bernhard Firner
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 * or visit http://www.gnu.org/licenses/gpl-2.0.html
 */

/*******************************************************************************
 * @file owl-traffic-generator.cpp
 * Generates traffic destined for an Owl Aggregator. This program allows users
 * to set the number of unique IDs, the packet interval, and the packet loss
 * rate for the data being sent to the aggregator.
 * The packet interval will be varied by .05%
 *
 * @author Bernhard Firner
 ******************************************************************************/

#include <sys/signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h> //For usleep

//TODO Create lib-cppsensor so that sockets don't need to be handled here.
#include <owl/sensor_connection.hpp>
#include <owl/sample_data.hpp>

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <stdexcept>

#include <random>

#include <queue>

//Handle interrupt signals to exit cleanly.
#include <signal.h>

using std::string;
using std::list;
using std::map;

//Global variable for the signal handler.
bool killed = false;
//Signal handler.
void handler(int signal) {
  psignal( signal, "Received signal ");
  if (killed) {
    std::cerr<<"Aborting.\n";
    // This is the second time we've received the interrupt, so just exit.
    exit(-1);
  }
  std::cerr<<"Shutting down...\n";
  killed = true;
}


int main(int ac, char** arg_vector) {
  if (ac != 5 ) {
    std::cerr<<"This program requires 5 arguments,"<<
      " the ip address and the port number of the owl aggregator,\n"<<
      " the number of unique ID's, the packet interval (in milliseconds),\n"<<
      " and the synthesized packet loss rate from 0 to 1.\n";
    return 0;
  }

  //Get the ip address and ports of the aggregation server
  std::string server_ip(arg_vector[1]);
  int server_port = atoi(arg_vector[2]);
  int total_ids   = atoi(arg_vector[3]);
  int interval    = atoi(arg_vector[4]);
  double p_loss   = atof(arg_vector[5]);

  //A structure to hold transmitter IDs and their scheduled transmission time
  struct Txer {
    int id;
    uint64_t tx_time;
    bool operator<(const Txer& other) const {
      return tx_time < other.tx_time;
    }
  };

  std::random_device rd;
  std::mt19937 gen(rd());


  //Initialize a list of the given number of tags
  std::priority_queue<Txer> tx_schedule;

  //Randomize starting from the current time to one interval away
  int64_t start_time = msecTime();
  std::uniform_int_distribution<> dis(start_time, start_time + interval);
  for (int i = 0; i < total_ids; ++i) {
    Txer txer;
    txer.id = i;
    txer.tx_time = dis(gen);
    tx_schedule.push(txer);
  }

  const double jitter = 0.05;
  std::uniform_int_distribution<> next_time(interval*(1.0-jitter), interval*(1.0+jitter));
  std::uniform_real_distribution<> rand_drop(0.0, 1.0);

  while (not killed) {
    SensorConnection agg(server_ip, server_port);

    //A try/catch block is set up to handle exception during quitting.
    try {
      while (agg and not killed) {
        //Build a sample out of the next message in the priority queue
        Txer txer = tx_schedule.top();
        tx_schedule.pop();
        //First see if we should randomly drop this packet
        if (rand_drop(gen) > p_loss) {
          SampleData sd;
          sd.physical_layer = 1;
          sd.tx_id = txer.id;
          sd.rx_id = 1;
          sd.rx_timestamp = txer.tx_time;
          sd.rss = -50.0;
          sd.valid = true;
          //Wait until the time to send arrives
          //TODO FIXME Should break long sleep up so that the program can be
          //cancelled by user intervention.
          int64_t time_diff = (txer.tx_time - msecTime());
          if (0 < time_diff) {
            usleep(1000*time_diff);
          }
          agg.send(sd);
        }
        //Schedule the next time
        txer.tx_time += next_time(gen);
        tx_schedule.push(txer);
      }
    }
    catch (std::runtime_error& re) {
      std::cerr<<"USB sensor layer error: "<<re.what()<<'\n';
    }
    //Try to reconnect to the server after losing the connection.
    //Sleep for 1 second, then try connecting to the server again.
    usleep(1000);
  }
  std::cerr<<"Exiting\n";
}


