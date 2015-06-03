Sonar application
=================

Illustrates radio propagation.

When receiving a character in [a-h], sends a broadcast message at a specific power.
When receiving a broadcast message, prints a report oon its serial link.

## Power specification
|'character'   | a   | b   | c   | d   | e   | f  | g | h |
|--------------|-----|-----|-----|-----|-----|----|---|---|
| power in dBm | -30 | -25 | -20 | -15 | -10 | -5 | 0 | 5 |

## Serial reporting
The report message written on the serial link contains:
  - source
  - dest
  - RSSI

Example:
'''
80ce;b012;12345
'''
