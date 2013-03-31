owl-traffic-generator
=====================

Generates traffic to an Owl aggregator for testing purposes. Features are:

Features
-----
 * Generates traffic destined for an Owl Aggregator.
 * Set numbers of unique IDs, the packet interval, and the packet loss rate of
   traffic.
 * The packet interval will be varied by .05% to provide "realistic" jitter.

Requirements
------------

This program requires cpp-owl-common and cpp-owl-sensor from https://github.com/OwlPlatform

Building
--------

cmake && make && sudo make install

The binary will be installed to the owl binary directory. If you skip the make
install step you can still find a copy of the binary in the ./bin directory of
this repository.

License
-------
 Copyright (c) 2013 Bernhard Firner
 All rights reserved.
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 or visit http://www.gnu.org/licenses/gpl-2.0.html
