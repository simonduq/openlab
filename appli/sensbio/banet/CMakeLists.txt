# This file is a part of openlab/sensbiotk
#
# Copyright (C) 2015  INRIA (Contact: sensbiotk@inria.fr)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if (${PLATFORM_HAS_TDMA})

    add_executable(banet_sink banet_sink)
    target_link_libraries(banet_sink mac_tdma platform)

    add_executable(banet_imu banet_imu sensor)
    target_link_libraries(banet_imu mac_tdma platform)

endif (${PLATFORM_HAS_TDMA})
