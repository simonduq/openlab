Sonar application
=================

Illustrates radio propagation.

Behaviour:
  - When receiving a character in [a-f], sends a broadcast message at a specific power.
  - When receiving the character 'h', print help.
  - When receiving the charecter 'r', turn off leds.
  - When receiving a broadcast message, prints a report on its serial link.

## Power specification
|'character'   |  a  |  b  |  c  |  d  |  e  |  f  |
|--------------|-----|-----|-----|-----|-----|-----|
| power in dBm | -17 | -12 | -7  | -3  |  0  |  3  |

## Serial reporting
The report message written on the serial link contains:
  - source
  - dest
  - RSSI

Example:
```
80ce;b012;12345
```
