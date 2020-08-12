# DreamLifter: a high-level semihosting for UMDF drivers

This the bare-minimum semihosting implementation for UMDF drivers. It is currently capable to load the Lumia 950 / Lumia 950 XL
USB-C controller arm32 driver on arm64 host. Actual IOCTLs are forwarded to kernel IO targets, and UCM requests are forwarded to
shims.

It might be feasible to semihost other UMDF drivers, but be aware a lot of structs and functions are not implemented yet. In addition
the host might have strong assumptions of arm32 architecture (i.e. data type lengths.)

## License 
MIT License