unsigned char find_end(const unsigned char *buf, 
    unsigned int start, unsigned int *end)
{
    unsigned int pos;
    for (pos = start; pos < start + 12U; pos++)
    {
        if (buf[pos] == '#') 
        {
            *end = pos;
            return 1U;
        }
        if (buf[pos] == 0U)
            return 0u;
    }
    return 0u;
}

// find_end(buf,i,&end);