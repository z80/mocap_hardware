
import math
import asyncio

async def queue_processor( queue, shared_data ):
    data_all = ""
    while True:
        data_batch = await queue.get()
        data_all.apppend( data_batch )

        # Search for "\r".
        index = data_all.index( "\r" )
        data = data_all[:index]
        data_all = data_all[(index+1):]

        # Convert data to numbers and quaternions.


def parse_data( data ):
    L = len( data )
    # Should be at least 2 uint32 numbers with 2 bytes per digit.
    # In total it is 4x2x2 = 16 bytes.
    if L < 16:
        return False

    channels_stri = data[8:16]
    channels_bit_mask = string_to_uint32( channels_stri )
    channels_qty = number_of_channels( channels_bit_mask )

    data_size = channels_qty * 16
    # Total size
    total_expected_size = data_size + 16

    if L != total_expected_size:
        return False

    total_channels_bit_mask = string_to_uint32( data )
    total_channels = get_channels( total_channels_bit_mask )

    channels = get_channels( channels_bit_mask )





def string_to_uint32( stri ):
    stri = stri[:8]
    number = int( stri, 16 )
    return stri



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


def get_channels( numer ):
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
    x = string_to_int_16( stri[4:] )
    y = string_to_int_16( stri[8:] )
    z = string_to_int_16( stri[12:] )

    L = math.sqrt( float(w*w + x*x + y*y + z*z) )

    w = float(w) / L
    x = float(x) / L
    y = float(y) / L
    z = float(z) / L

    return {"w":w, "x": x, "y": y, "z": z}



