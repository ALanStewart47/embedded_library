

/* Parse an unsigned decimal integer from buf[*pos].
 * Stop at first non-digit.
 * Advances *pos.
 * Returns 1 on success, 0 on error.
 */
int parse_uint_decimal(unsigned char *buf, unsigned int *pos, 
                        unsigned int end, unsigned int max_digits,
                         unsigned int *out)
{
    unsigned int v      = 0;
    unsigned int digits = 0;

    while (*pos < end)
    {
        unsigned char ch = buf[*pos];

        if (ch < '0' || ch > '9')
            break;

        v = v * 10 + (unsigned int)(ch -' 0');
        digits++;

        if (digits > max_digits)
            return 0;

        (*pos)++;
    }

    if (digits == 0)
        return 0;

    *out = v;
    return 1;
}
