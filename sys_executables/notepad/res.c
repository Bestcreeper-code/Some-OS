
void intToStr(int num, char* str) {
    int i = 0;

    // Handle zero explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Extract digits in reverse order
    while (num > 0) {
        int digit = num % 10;
        str[i++] = digit + '0'; // Convert digit to char
        num /= 10;
    }
    str[i] = '\0';

    // Reverse the string because digits are in reverse
    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - 1 - j];
        str[i - 1 - j] = tmp;
    }
}
