// Compute the MODBUS RTU CRC
// The following code is a C++ implementation of the CRC calculation for MODBUS RTU.
buf = [7, 16, 0, 0, 0, 6, 12, 0, 11, 0, 2, 0, 1, 0, 11, 0, 0, 0, 1]

UInt16 ModRTU_CRC(byte[] buf, int len)
{
  UInt16 crc = 0xFFFF;
  
  for (int pos = 0; pos < len; pos++) {
    crc ^= (UInt16)buf[pos];          // XOR byte into least sig. byte of crc
  
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  print crc;  
}