# Audio Volume Control via Hardware

A Hardware controller for controlling volume levels on a output device or audio session level.
It is designed for up to 6 volume sliders/knobs but may be extended in the future.

> **_INFORMATION_**: This project overrides session and device volume levels and therefore does not work with audio ducking.

## Communication

There are two ways the controller and device can communicate.
1. The Device requests specific data (polling) and the controller responds
2. The controller has detected changed data and notifies the device

### Basic structure

- Slider IDs range from 0 to 5 (3 Bit)
- Slider Values range from 0 to 1023 (10 Bit)

#### Request
```
0xxx xxxx 1yyy yyyy
 \         \________ request data
  \_________________ message type
```

### Response
```
1xxx xxxx [data]
 \         \_____ the corresponding response data (size dependent on message type)
  \______________ the message type of the corresponding request
```

### 0x01 Capabilities Request
| Message Type | Description                                           |
|--------------|-------------------------------------------------------|
| 0x01         | Sent by the Device to request controller capabilities |
```
Request:
0000 0001 1000 0000

Response:
1000 0001 xxx0 0000
          \_________ amount of connected sliders (0 - 6)
```

### 0x02 Single Slider Request
| Message Type | Description                                             |
|--------------|---------------------------------------------------------|
| 0x02         | Sent by the Device to request data for a single slider. |
```
Request:
0000 0010 1000 0xxx
                \___ slider index (0 - 5)

Response:
1000 0010 0xxx yyyy 10yy yyyy
           \   \      \_______ lower 6 bits of slider value
            \   \_____________ upper 4 bits of slider value
             \________________ slider index (0 - 5)
```

### 0x03 Multi Slider Request
| Message Type | Description                                           |
|--------------|-------------------------------------------------------|
| 0x03         | Sent by the Device to request data for a all sliders. |
```
Request:
0000 0011 1000 0000

Response:
1000 0011 [ 0xxx yyyy 10yy yyyy ] * slider count
             \   \      \_______ lower 6 bits of slider value
              \   \_____________ upper 4 bits of slider value
               \________________ slider index (0 - 5)
```

### 0x10 Data change notification
| Message Type | Description                                           |
|--------------|-------------------------------------------------------|
| 0x10         | Sent by the Controller to notify the device of new available data.<br>The device will only send this once and not again until any request<br>was made by the device for the changed slider (single or multi). |
```
0001 0000 1000 0xxx
                \___ slider index (0 - 5)
```
