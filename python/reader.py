#!/usr/bin/python3
#
# Reader for FRC CAN messages, produces a dataframe.
#
# Usage:
#
#   python3 reader.py infile
#
# See dpkt/examples/print_packets.py
# See docs.wpilib.org/en/stable/docs/software/can-devices/can-addressing.html
#
# TODO: add more columns

import bitstring
import datetime
import dpkt
import sys
import pandas as pd

def main(infile):
    with open(infile, "rb") as f:
        r = dpkt.pcap.Reader(f)
        if r.datalink() != 190:
            sys.exit(
                f"Fatal: Network {r.datalink()} is not the required CAN type"
            )
        #print("time(s)       type mfr  cls  idx  num  data")
        #print("------------- ---- ---- ---- ---- ---- ------------------")
        rows = {}
        try:
            for ts, buf in r:
                if len(buf) < 8 or len(buf) > 16:
                    continue # id is exactly 8 bytes, payload is 0-8 bytes

                record = bitstring.BitArray(buf)
                canid, data_length, data = record.unpack("uintle32, uint8, pad24, bits")

                # eff = (canid & 0x80000000) >> 31
                # rtr = (canid & 0x40000000) >> 30
                # err = (canid & 0x20000000) >> 29
                # canid = canid & 0x1FFFFFFF

                device_type = (canid & 0x1F000000) >> 24
                manufacturer = (canid & 0x00FF0000) >> 16
                api_class = (canid & 0x0000FC00) >> 10
                api_index = (canid & 0x000003C0) >> 6
                device_number = canid & 0x0000003F

                #print(
                #    f"{ts:13.6f} {device_type:>4} {manufacturer:>4} {api_class:>4} {api_index:>4} {device_number:>4} {data}"
                #)
                if ts not in rows:
                    rows[ts] = {"timestamp":ts}
                rows[ts]['device type'] = device_type
                rows[ts]['manufacturer'] = manufacturer
                rows[ts]['api class'] = api_class
                rows[ts]['api index'] = api_index
                rows[ts]['device number'] = device_number
                #rows[ts]['data'] = data
        except dpkt.dpkt.NeedData:
            #print("end of file?")
            pass

        df = pd.DataFrame.from_records(list(rows.values()))
        df.set_index("timestamp", inplace=True)
        df.pad(inplace=True) # last obs carried forward
        print(df)

if __name__ == "__main__":
    infile = sys.argv[1]
    main(infile)
