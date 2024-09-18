
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
                    print( "Something went wrong sending data." )

        #semaphore.release()
        print( "shared data: ", shared_data )


def parse_data( data, shared_data ):
    L = len( data )
    # Should be at least 2 uint32 numbers with 2 bytes per digit.
    # In total it is 4x2x2 = 16 bytes.
    if L < 20:
        return False

    voltage_adc = string_to_uint16( data[:4] )
    voltage = float(voltage_adc) * (2.0 * 3.3 / 4095.0)
    shared_data['v_batt'] = voltage


    channels_stri = data[12:20]
    channels_bit_mask = string_to_uint32( channels_stri )
    channels_qty = number_of_channels( channels_bit_mask )

    data_size = channels_qty * 16
    # Total size
    total_expected_size = data_size + 20

    if L != total_expected_size:
        return False

    total_channels_bit_mask = string_to_uint32( data[4:12] )

    total_channels = get_channels( total_channels_bit_mask )

    channels = get_channels( channels_bit_mask )

    for channel_ind in range(channels_qty):
        q_data_ind = 20 + 16*channel_ind
        q_data = data[q_data_ind:(q_data_ind+16)]

        q = string_to_quaternion( q_data )
        
        channel_id = channels[channel_ind]
        shared_data[channel_id] = q

    return True





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


def number_of_channels( number ):
    accum = 0

    for i in range(32):
        bit = 1 << i
        exists = (number & bit) != 0
        if exists:
            accum += 1

    return accum


def get_channels( number ):
    channels = []
    for i in range(32):
        bit = 1 << i
        exists = (number & bit) != 0
        if exists:
            channels.append( i )

    return channels

    return accum



def string_to_quaternion( stri ):
    stri = stri[:16]
    w = string_to_int16( stri )
    x = string_to_int16( stri[4:] )
    y = string_to_int16( stri[8:] )
    z = string_to_int16( stri[12:] )

    L = math.sqrt( float(w*w + x*x + y*y + z*z) )

    w = float(w) / L
    x = float(x) / L
    y = float(y) / L
    z = float(z) / L

    return {"w":w, "x": x, "y": y, "z": z}



