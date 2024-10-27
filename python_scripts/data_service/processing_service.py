
import math
import json
import asyncio

async def processing_service( queue, clients_list, shared_data, semaphore ):
    print( "#" * 300 )
    data_all = ""
    #import pdb
    #pdb.set_trace()
    while True:
        #print( "Waiting for data" )
        data_batch = await queue.get()
        #print( "Received data", data_batch )
        data_all += data_batch

        # Search for "\r".
        index = data_all.find( "\r" )
        if index < 0:
            continue

        data = data_all[:index]
        data_all = data_all[(index+1):]

        # Convert data to numbers and quaternions.
        #print( "processing: ", data )
        ret = parse_data( data, shared_data )
        #print( "processed: ", ret )

        if ret:
            qty = len(clients_list)
            if qty > 0:
                stri = json.dumps( shared_data )
                tasks = [ client.send( stri ) for client in clients_list ]
                try:
                    await asyncio.gather(*tasks)
                except:
                    print( "Something went wrong while sending data." )

        #semaphore.release()
        #print( "shared data: ", shared_data )


def parse_data( data, shared_data ):
    L = len( data )
    # First, batt voltage as uint16_t -> 4 bytes
    # Second, number of IMUs detected -> 8 bytes.
    # 0..32 IMU Quaternions, each number in the quaternion is 4 bytes.
    # Last, there is a CRC8 expressed as 2 bytes.
    # In total data size = 4 + 8 + N*4*4 + 2
    #shared_data["L"]   = L
    if L < 14:
        return False

    ok = check_crc8( data )
    if ( not ok ):
        print( "crc ok: ", ok )
        return False


    #import pdb
    #pdb.set_trace()

    voltage_adc = string_to_uint16( data[:4] )
    voltage = float(voltage_adc) * (2.0 * 3.3 / 4095.0)
    shared_data['v_batt'] = voltage

    imu_bits = string_to_uint32( data[4:12] )
    indices = bits_to_numbers( imu_bits )
    readings_qty = len( indices )

    shared_data["qty"] = readings_qty

    quats = {}
    for channel_ind in range(readings_qty):
        q_data_ind = 12 + 16*channel_ind
        q_data = data[q_data_ind:(q_data_ind+16)]

        q = string_to_quaternion( q_data, data )

        quat_ind = indices[channel_ind]
        
        quats[quat_ind] = q
    shared_data["quats"] = quats

    #print( "q[28]: ", quats[28], "q[14]: ", quats[14] )
    #ind28 = 6 + 16*28
    #print( "q[28]: ", quats[28], "stri: ", data[ind28:(ind28+16)] )
    #print( shared_data )

    return True



def check_crc8( data ):
    #import pdb
    #pdb.set_trace()
    qty = len( data )
    qty = qty - 2

    crc8 = 0
    for i in range(qty):
        byte_stri = data[i]
        byte = byte_to_uint8( byte_stri )

        crc8 = update_crc8( byte, crc8 )

    # Read the transmitted CRC8
    transmitted_crc8 = string_to_uint8( data[-2:] )

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


def string_to_uint32( stri ):
    stri = stri[:8]
    number = int( stri, 16 )
    return number




def string_to_uint16( stri ):
    stri = stri[:4]
    number = int( stri, 16 )

    return number

    

def string_to_int16( stri ):
    stri = stri[:4]
    number = int( stri, 16 )
    if number > 32767:
        number = number - 65536

    return number


def string_to_uint8( stri ):
    stri = stri[:2]
    number = int( stri, 16 )
    return number


def byte_to_uint8( stri ):
    number = ord( stri )
    return number


def bits_to_numbers( bit_mask ):
    indices = []
    for i in range(32):
        bit = 1 << i
        exists = (bit_mask & bit) != 0
        if exists:
            indices.append(i)

    return indices




def string_to_quaternion( stri, all_data ):
    stri = stri[:16]
    w = string_to_int16( stri )
    x = string_to_int16( stri[4:] )
    y = string_to_int16( stri[8:] )
    z = string_to_int16( stri[12:] )

    L = math.sqrt( float(w*w + x*x + y*y + z*z) )

    if ( L < 0.1 ):
        import pdb
        pdb.set_trace()

    w = float(w) / L
    x = float(x) / L
    y = float(y) / L
    z = float(z) / L

    return {"w": w, "x": x, "y": y, "z": z}



