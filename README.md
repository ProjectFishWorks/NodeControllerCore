# Node Controller Core

The node controller core a library to implement methods that are shared between all device firmware running on a node controller. At this time, these methods are all related to CAN Bus communication.

## CAN Bus Message Format

### Payload (64 bits)
The payload field contains the data that is being sent or received, up to 8 bytes (64 bit). The format of this data depends on what is required for a given message type. Data is often a 32bit float or a interger.

### Indentifer (29 bits)
Since we are representing several pieces of information in the indentifier, we used the extended ID format (CAN 2.0B). The format of the indentifer field is as follows:

```
(NNNN)(NNNN)(MMMM)(MMMM)(MMMM)(MMMM)(RRRR)(L)
```

* N = Node ID: Unique ID of the current node

* M = Message ID: Unique ID of the message type being sent

* L = Log Message Flag: 0 = base station will log message, 1 = base station will not log message.
