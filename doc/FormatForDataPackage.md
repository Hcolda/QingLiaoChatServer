# Data package
## Format
Format: `length + requestID + data type + sequence + hash + binary data + double '\0'`

| Type | Name | Value | Comment |
| :---: | :---: | :---: | :---: |
| int | length | Length of data package |  |
| long long | requestID |  |  |
| int | type | Default 0 | Type of data package, `1 for text, 2 for binary, 3 for file stream, 4 for heartbeat package` |
| int | sequence | Default -1 | Valid if data package is splitted |
| std::size_t | verifycode | Hash | Hash for verification |
| char | data | Binary data | |
| char | ___ | double '\0' | End of data package |
