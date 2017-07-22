int http_str2int(char* p) {
    int result = 0;
    char c;
    while ((c = *p++)) {
        result = result * 10 + (c-'0');
    }
    return result;
}

int short2str(short n, char* p) {
    if (n == 0) {
        *p = '\0';
        return 0;
    } else {
        if (short2str(n/10, (p+1)) != 0)
            return -1;
        *p = (char)(n % 10 + '0');
    }
    return 0;
}
