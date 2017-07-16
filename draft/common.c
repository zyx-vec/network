int http_str2int(char* p) {
    int result = 0;
    char c;
    while ((c = *p++)) {
        result = result * 10 + (c-'0');
    }
    return result;
}
