void reply_u32 (uint8_t command, uint8_t channel, uint32_t value)
{
    uint8_t tx[13], reverse[10], count = 0U, index = 0U;
    tx[index++] = command;
    tx[index++] = (uint8_t)(channel + ('a' - 'A'));
    do { reverse[count++] = (uint8_t)('0' + value % 10U); value / 10;}
    while(value != 0U);
    while(count != 0U) tx[index++] = reverse[--count];
    //send
}

//13  10 is magic number,
//core is % and / 