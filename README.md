# simple-vlc
a simple visible light communication

### Build and flash

```bash
# flash transmit
pio run -e transmitter            # Build only
pio run -e transmitter -t upload  # Build and upload to the board

# flash repeater
pio run -e repeater             # Build only
pio run -e repeater -t upload   # Build and upload to the board

# flash receive
pio run -e receiver             # Build only
pio run -e receiver -t upload   # Build and upload to the board
```

### TO-DO
- Make an error detection [done]
- Calculate bit error rate [done]
- Make a repeater [done]