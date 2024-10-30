
import math
import json
import asyncio

async def processing_service( queue, clients_list, shared_data, semaphore ):
    print( "#" * 300 )
    data_all = bytearray()
    #import pdb
    #pdb.set_trace()
    while True:
        #print( "Waiting for data" )
        data_batch = await queue.get()
        #print( "Received data", data_batch )
        data_all += data_batch

        # Search for a magic byte "0xAA".
        index = data_all.find( 0xAA )
        if index < 0:
            continue

        data = data_all[(index+1):]
        # It is followed by 2 bytes voltage and 4 bytes bit mask and a checksum.
        # So, the data tail length should be at least 7 bytes.
        length = len(data)
        if length < 7:
            data_all = data_all[index:]
            continue

        # Convert data to numbers and quaternions.
        #print( "processing: ", data )
        used_bytes_qty = parse_data( data, shared_data )
        #print( "processed: ", ret )

        if used_bytes_qty > 0:
            next_index = index + used_bytes_qty
            data_all = data_all[next_index:]
            qty = len(clients_list)
            if qty > 0:
                stri = json.dumps( shared_data )
                tasks = [ client.send( stri ) for client in clients_list ]
                try:
                    await asyncio.gather(*tasks)
                except:
                    print( "Something went wrong while sending data." )

        elif used_bytes_qty == -1:
            data_all = data_all[index:]

        elif used_bytes_qty == -2:
            data_all = data_all[(index+1):]
            

        #semaphore.release()
        #print( "shared data: ", shared_data )


def parse_data( data, shared_data ):
    L = len( data )
    # First, batt voltage as uint16_t -> 4 bytes
    # Second, number of IMUs detected -> 8 bytes.
    # 0..32 IMU Quaternions, each number in the quaternion is 4 bytes.
    # Last, there is a CRC8 expressed as 2 bytes.
    # In total data size = 2 + 4 + N*8 + 1
    #shared_data["L"]   = L

    voltage_adc = array_to_uint16( data[:2] )
    voltage = float(voltage_adc) * (2.0 * 3.3 / 4095.0)

    imu_bits = array_to_uint32( data[2:6] )
    indices = bits_to_numbers( imu_bits )
    readings_qty = len( indices )

    # Expected data size
    expected_total_bytes = 2 + 4 + 1 + readings_qty * 8
    if L < expected_total_bytes:
        return -1

    #import pdb
    #pdb.set_trace()

    ok = check_crc8( data, expected_total_bytes )
    if ( not ok ):
        print( "crc ok: ", ok )
        return -2

    shared_data['v_batt'] = voltage
    shared_data["qty"]    = readings_qty

    quats = {}
    for channel_ind in range(readings_qty):
        q_data_ind = 6 + 8*channel_ind
        q_data = data[q_data_ind:(q_data_ind+8)]

        q = array_to_quaternion( q_data )

        quat_ind = indices[channel_ind]
        
        quats[quat_ind] = q
    shared_data["quats"] = quats

    #print( "q[28]: ", quats[28], "q[14]: ", quats[14] )
    #ind28 = 6 + 16*28
    #print( "q[28]: ", quats[28], "stri: ", data[ind28:(ind28+16)] )
    #print( shared_data )

    return expected_total_bytes



def check_crc8( data, bytes_qty ):
    """
    bytes_qty includes the crc8 itself at the end.
    Total data length may be more than that.
    """
    #import pdb
    #pdb.set_trace()
    qty = bytes_qty - 1

    crc8 = 0
    for i in range(qty):
        byte = data[i]
        crc8 = update_crc8( byte, crc8 )

    # Read the transmitted CRC8
    transmitted_crc8 = data[qty]

    result_ok = (crc8 == transmitted_crc8)
    return result_ok



def update_crc8( byte, crc8 ):
    crc8 ^= byte
    for j in range(8):
        if (crc8 & 0x80) != 0:
            crc8 = crc8 << 1
            crc8 = crc8 ^ 0x07
        else:
            crc8 = crc8 << 1

        crc8 = crc8 & 0xFF

    return crc8


def array_to_uint32( stri ):
    stri = stri[:4]
    number = 0
    multiplier = 1
    for i in range(4):
        v = stri[i]
        v *= multiplier
        number += v
        multiplier = multiplier << 8
    return number




def array_to_uint16( stri ):
    stri = stri[:2]
    number = 0
    multiplier = 1
    for i in range(2):
        v = stri[i]
        v *= multiplier
        number += v
        multiplier = multiplier << 8
    return number


    

def array_to_int16( stri ):
    stri = stri[:2]
    number = 0
    multiplier = 1
    for i in range(2):
        v = stri[i]
        v *= multiplier
        number += v
        multiplier = multiplier << 8

    if number > 32767:
        number = number - 65536

    return number




def bits_to_numbers( bit_mask ):
    indices = []
    for i in range(32):
        bit = 1 << i
        exists = (bit_mask & bit) != 0
        if exists:
            indices.append(i)

    return indices




def array_to_quaternion( stri ):
    stri = stri[:8]
    w = array_to_int16( stri )
    x = array_to_int16( stri[2:] )
    y = array_to_int16( stri[4:] )
    z = array_to_int16( stri[6:] )

    L = math.sqrt( float(w*w + x*x + y*y + z*z) )

    if ( L < 0.1 ):
        import pdb
        pdb.set_trace()

    w = float(w) / L
    x = float(x) / L
    y = float(y) / L
    z = float(z) / L

    return {"w": w, "x": x, "y": y, "z": z}



