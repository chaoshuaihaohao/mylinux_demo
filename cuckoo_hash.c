//用C语言实现。CRC32作为hash函数，输入key值是48bit，布谷鸟哈希桶深度是512，桶的个数是2.给定输入文件trace.txt，trace.txt的每行内容格式为key=0x1234567890ab。解析trace.txt文件，将0x1234567890ab作为hash输入，计算出pos值，然后根据pos值和布谷鸟哈希算法插入key到桶中，需要实现布谷鸟hash的kick和rehash机制。然后将最终桶中的内容写到文件hw.txt中

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define TABLE_SIZE 2048
#define BUCKET_COUNT 2
#define SEED1 0x1234ABCD
#define SEED2 0x98765432

typedef struct {
	uint8_t key[6];
	uint8_t value[2];
	int occupied;
} Bucket;

typedef struct {
	Bucket buckets[BUCKET_COUNT][TABLE_SIZE];
	uint32_t seed[BUCKET_COUNT];
	uint32_t mux_seed[BUCKET_COUNT];
} CuckooHashTable;

typedef struct {
	uint8_t key[6];
	uint8_t value[2];
} Rule;

struct stream {
	uint8_t kick[TABLE_SIZE][6];
	uint32_t count;
};

uint32_t crc32_hash(const uint8_t *key, uint32_t seed)
{
	uint32_t crc = 0;
        uint8_t c[32] = { 0 };
        uint8_t q[32] = { 0 };
        uint8_t in[48] = { 0 };
        int i, j;

        for (i = 0; i < 32; i++)
                q[i] = (seed >> i) & 0x1;

        for (i = 0; i < 6; i++)
                for (j = 0; j < 8; j++)
                        in[i * 8 + j] = (key[i] >> j) & 0x1;

	c[0] =
	    q[0] ^ q[8] ^ q[9] ^ q[10] ^ q[12] ^ q[13] ^ q[14] ^ q[15] ^ q[16] ^
	    q[18] ^ q[21] ^ q[28] ^ q[29] ^ q[31] ^ in[0] ^ in[6] ^ in[9] ^
	    in[10] ^ in[12] ^ in[16] ^ in[24] ^ in[25] ^ in[26] ^ in[28] ^
	    in[29] ^ in[30] ^ in[31] ^ in[32] ^ in[34] ^ in[37] ^ in[44] ^
	    in[45] ^ in[47];
	c[1] =
	    q[0] ^ q[1] ^ q[8] ^ q[11] ^ q[12] ^ q[17] ^ q[18] ^ q[19] ^ q[21] ^
	    q[22] ^ q[28] ^ q[30] ^ q[31] ^ in[0] ^ in[1] ^ in[6] ^ in[7] ^
	    in[9] ^ in[11] ^ in[12] ^ in[13] ^ in[16] ^ in[17] ^ in[24] ^ in[27]
	    ^ in[28] ^ in[33] ^ in[34] ^ in[35] ^ in[37] ^ in[38] ^ in[44] ^
	    in[46] ^ in[47];
	c[2] =
	    q[0] ^ q[1] ^ q[2] ^ q[8] ^ q[10] ^ q[14] ^ q[15] ^ q[16] ^ q[19] ^
	    q[20] ^ q[21] ^ q[22] ^ q[23] ^ q[28] ^ in[0] ^ in[1] ^ in[2] ^
	    in[6] ^ in[7] ^ in[8] ^ in[9] ^ in[13] ^ in[14] ^ in[16] ^ in[17] ^
	    in[18] ^ in[24] ^ in[26] ^ in[30] ^ in[31] ^ in[32] ^ in[35] ^
	    in[36] ^ in[37] ^ in[38] ^ in[39] ^ in[44];
	c[3] =
	    q[1] ^ q[2] ^ q[3] ^ q[9] ^ q[11] ^ q[15] ^ q[16] ^ q[17] ^ q[20] ^
	    q[21] ^ q[22] ^ q[23] ^ q[24] ^ q[29] ^ in[1] ^ in[2] ^ in[3] ^
	    in[7] ^ in[8] ^ in[9] ^ in[10] ^ in[14] ^ in[15] ^ in[17] ^ in[18] ^
	    in[19] ^ in[25] ^ in[27] ^ in[31] ^ in[32] ^ in[33] ^ in[36] ^
	    in[37] ^ in[38] ^ in[39] ^ in[40] ^ in[45];
	c[4] =
	    q[2] ^ q[3] ^ q[4] ^ q[8] ^ q[9] ^ q[13] ^ q[14] ^ q[15] ^ q[17] ^
	    q[22] ^ q[23] ^ q[24] ^ q[25] ^ q[28] ^ q[29] ^ q[30] ^ q[31] ^
	    in[0] ^ in[2] ^ in[3] ^ in[4] ^ in[6] ^ in[8] ^ in[11] ^ in[12] ^
	    in[15] ^ in[18] ^ in[19] ^ in[20] ^ in[24] ^ in[25] ^ in[29] ^
	    in[30] ^ in[31] ^ in[33] ^ in[38] ^ in[39] ^ in[40] ^ in[41] ^
	    in[44] ^ in[45] ^ in[46] ^ in[47];
	c[5] =
	    q[3] ^ q[4] ^ q[5] ^ q[8] ^ q[12] ^ q[13] ^ q[21] ^ q[23] ^ q[24] ^
	    q[25] ^ q[26] ^ q[28] ^ q[30] ^ in[0] ^ in[1] ^ in[3] ^ in[4] ^
	    in[5] ^ in[6] ^ in[7] ^ in[10] ^ in[13] ^ in[19] ^ in[20] ^ in[21] ^
	    in[24] ^ in[28] ^ in[29] ^ in[37] ^ in[39] ^ in[40] ^ in[41] ^
	    in[42] ^ in[44] ^ in[46];
	c[6] =
	    q[4] ^ q[5] ^ q[6] ^ q[9] ^ q[13] ^ q[14] ^ q[22] ^ q[24] ^ q[25] ^
	    q[26] ^ q[27] ^ q[29] ^ q[31] ^ in[1] ^ in[2] ^ in[4] ^ in[5] ^
	    in[6] ^ in[7] ^ in[8] ^ in[11] ^ in[14] ^ in[20] ^ in[21] ^ in[22] ^
	    in[25] ^ in[29] ^ in[30] ^ in[38] ^ in[40] ^ in[41] ^ in[42] ^
	    in[43] ^ in[45] ^ in[47];
	c[7] =
	    q[0] ^ q[5] ^ q[6] ^ q[7] ^ q[8] ^ q[9] ^ q[12] ^ q[13] ^ q[16] ^
	    q[18] ^ q[21] ^ q[23] ^ q[25] ^ q[26] ^ q[27] ^ q[29] ^ q[30] ^
	    q[31] ^ in[0] ^ in[2] ^ in[3] ^ in[5] ^ in[7] ^ in[8] ^ in[10] ^
	    in[15] ^ in[16] ^ in[21] ^ in[22] ^ in[23] ^ in[24] ^ in[25] ^
	    in[28] ^ in[29] ^ in[32] ^ in[34] ^ in[37] ^ in[39] ^ in[41] ^
	    in[42] ^ in[43] ^ in[45] ^ in[46] ^ in[47];
	c[8] =
	    q[1] ^ q[6] ^ q[7] ^ q[12] ^ q[15] ^ q[16] ^ q[17] ^ q[18] ^ q[19] ^
	    q[21] ^ q[22] ^ q[24] ^ q[26] ^ q[27] ^ q[29] ^ q[30] ^ in[0] ^
	    in[1] ^ in[3] ^ in[4] ^ in[8] ^ in[10] ^ in[11] ^ in[12] ^ in[17] ^
	    in[22] ^ in[23] ^ in[28] ^ in[31] ^ in[32] ^ in[33] ^ in[34] ^
	    in[35] ^ in[37] ^ in[38] ^ in[40] ^ in[42] ^ in[43] ^ in[45] ^
	    in[46];
	c[9] =
	    q[2] ^ q[7] ^ q[8] ^ q[13] ^ q[16] ^ q[17] ^ q[18] ^ q[19] ^ q[20] ^
	    q[22] ^ q[23] ^ q[25] ^ q[27] ^ q[28] ^ q[30] ^ q[31] ^ in[1] ^
	    in[2] ^ in[4] ^ in[5] ^ in[9] ^ in[11] ^ in[12] ^ in[13] ^ in[18] ^
	    in[23] ^ in[24] ^ in[29] ^ in[32] ^ in[33] ^ in[34] ^ in[35] ^
	    in[36] ^ in[38] ^ in[39] ^ in[41] ^ in[43] ^ in[44] ^ in[46] ^
	    in[47];
	c[10] =
	    q[0] ^ q[3] ^ q[10] ^ q[12] ^ q[13] ^ q[15] ^ q[16] ^ q[17] ^ q[19]
	    ^ q[20] ^ q[23] ^ q[24] ^ q[26] ^ in[0] ^ in[2] ^ in[3] ^ in[5] ^
	    in[9] ^ in[13] ^ in[14] ^ in[16] ^ in[19] ^ in[26] ^ in[28] ^ in[29]
	    ^ in[31] ^ in[32] ^ in[33] ^ in[35] ^ in[36] ^ in[39] ^ in[40] ^
	    in[42];
	c[11] =
	    q[0] ^ q[1] ^ q[4] ^ q[8] ^ q[9] ^ q[10] ^ q[11] ^ q[12] ^ q[15] ^
	    q[17] ^ q[20] ^ q[24] ^ q[25] ^ q[27] ^ q[28] ^ q[29] ^ q[31] ^
	    in[0] ^ in[1] ^ in[3] ^ in[4] ^ in[9] ^ in[12] ^ in[14] ^ in[15] ^
	    in[16] ^ in[17] ^ in[20] ^ in[24] ^ in[25] ^ in[26] ^ in[27] ^
	    in[28] ^ in[31] ^ in[33] ^ in[36] ^ in[40] ^ in[41] ^ in[43] ^
	    in[44] ^ in[45] ^ in[47];
	c[12] =
	    q[1] ^ q[2] ^ q[5] ^ q[8] ^ q[11] ^ q[14] ^ q[15] ^ q[25] ^ q[26] ^
	    q[30] ^ q[31] ^ in[0] ^ in[1] ^ in[2] ^ in[4] ^ in[5] ^ in[6] ^
	    in[9] ^ in[12] ^ in[13] ^ in[15] ^ in[17] ^ in[18] ^ in[21] ^ in[24]
	    ^ in[27] ^ in[30] ^ in[31] ^ in[41] ^ in[42] ^ in[46] ^ in[47];
	c[13] =
	    q[0] ^ q[2] ^ q[3] ^ q[6] ^ q[9] ^ q[12] ^ q[15] ^ q[16] ^ q[26] ^
	    q[27] ^ q[31] ^ in[1] ^ in[2] ^ in[3] ^ in[5] ^ in[6] ^ in[7] ^
	    in[10] ^ in[13] ^ in[14] ^ in[16] ^ in[18] ^ in[19] ^ in[22] ^
	    in[25] ^ in[28] ^ in[31] ^ in[32] ^ in[42] ^ in[43] ^ in[47];
	c[14] =
	    q[1] ^ q[3] ^ q[4] ^ q[7] ^ q[10] ^ q[13] ^ q[16] ^ q[17] ^ q[27] ^
	    q[28] ^ in[2] ^ in[3] ^ in[4] ^ in[6] ^ in[7] ^ in[8] ^ in[11] ^
	    in[14] ^ in[15] ^ in[17] ^ in[19] ^ in[20] ^ in[23] ^ in[26] ^
	    in[29] ^ in[32] ^ in[33] ^ in[43] ^ in[44];
	c[15] =
	    q[0] ^ q[2] ^ q[4] ^ q[5] ^ q[8] ^ q[11] ^ q[14] ^ q[17] ^ q[18] ^
	    q[28] ^ q[29] ^ in[3] ^ in[4] ^ in[5] ^ in[7] ^ in[8] ^ in[9] ^
	    in[12] ^ in[15] ^ in[16] ^ in[18] ^ in[20] ^ in[21] ^ in[24] ^
	    in[27] ^ in[30] ^ in[33] ^ in[34] ^ in[44] ^ in[45];
	c[16] =
	    q[1] ^ q[3] ^ q[5] ^ q[6] ^ q[8] ^ q[10] ^ q[13] ^ q[14] ^ q[16] ^
	    q[19] ^ q[21] ^ q[28] ^ q[30] ^ q[31] ^ in[0] ^ in[4] ^ in[5] ^
	    in[8] ^ in[12] ^ in[13] ^ in[17] ^ in[19] ^ in[21] ^ in[22] ^ in[24]
	    ^ in[26] ^ in[29] ^ in[30] ^ in[32] ^ in[35] ^ in[37] ^ in[44] ^
	    in[46] ^ in[47];
	c[17] =
	    q[2] ^ q[4] ^ q[6] ^ q[7] ^ q[9] ^ q[11] ^ q[14] ^ q[15] ^ q[17] ^
	    q[20] ^ q[22] ^ q[29] ^ q[31] ^ in[1] ^ in[5] ^ in[6] ^ in[9] ^
	    in[13] ^ in[14] ^ in[18] ^ in[20] ^ in[22] ^ in[23] ^ in[25] ^
	    in[27] ^ in[30] ^ in[31] ^ in[33] ^ in[36] ^ in[38] ^ in[45] ^
	    in[47];
	c[18] =
	    q[3] ^ q[5] ^ q[7] ^ q[8] ^ q[10] ^ q[12] ^ q[15] ^ q[16] ^ q[18] ^
	    q[21] ^ q[23] ^ q[30] ^ in[2] ^ in[6] ^ in[7] ^ in[10] ^ in[14] ^
	    in[15] ^ in[19] ^ in[21] ^ in[23] ^ in[24] ^ in[26] ^ in[28] ^
	    in[31] ^ in[32] ^ in[34] ^ in[37] ^ in[39] ^ in[46];
	c[19] =
	    q[0] ^ q[4] ^ q[6] ^ q[8] ^ q[9] ^ q[11] ^ q[13] ^ q[16] ^ q[17] ^
	    q[19] ^ q[22] ^ q[24] ^ q[31] ^ in[3] ^ in[7] ^ in[8] ^ in[11] ^
	    in[15] ^ in[16] ^ in[20] ^ in[22] ^ in[24] ^ in[25] ^ in[27] ^
	    in[29] ^ in[32] ^ in[33] ^ in[35] ^ in[38] ^ in[40] ^ in[47];
	c[20] =
	    q[0] ^ q[1] ^ q[5] ^ q[7] ^ q[9] ^ q[10] ^ q[12] ^ q[14] ^ q[17] ^
	    q[18] ^ q[20] ^ q[23] ^ q[25] ^ in[4] ^ in[8] ^ in[9] ^ in[12] ^
	    in[16] ^ in[17] ^ in[21] ^ in[23] ^ in[25] ^ in[26] ^ in[28] ^
	    in[30] ^ in[33] ^ in[34] ^ in[36] ^ in[39] ^ in[41];
	c[21] =
	    q[1] ^ q[2] ^ q[6] ^ q[8] ^ q[10] ^ q[11] ^ q[13] ^ q[15] ^ q[18] ^
	    q[19] ^ q[21] ^ q[24] ^ q[26] ^ in[5] ^ in[9] ^ in[10] ^ in[13] ^
	    in[17] ^ in[18] ^ in[22] ^ in[24] ^ in[26] ^ in[27] ^ in[29] ^
	    in[31] ^ in[34] ^ in[35] ^ in[37] ^ in[40] ^ in[42];
	c[22] =
	    q[0] ^ q[2] ^ q[3] ^ q[7] ^ q[8] ^ q[10] ^ q[11] ^ q[13] ^ q[15] ^
	    q[18] ^ q[19] ^ q[20] ^ q[21] ^ q[22] ^ q[25] ^ q[27] ^ q[28] ^
	    q[29] ^ q[31] ^ in[0] ^ in[9] ^ in[11] ^ in[12] ^ in[14] ^ in[16] ^
	    in[18] ^ in[19] ^ in[23] ^ in[24] ^ in[26] ^ in[27] ^ in[29] ^
	    in[31] ^ in[34] ^ in[35] ^ in[36] ^ in[37] ^ in[38] ^ in[41] ^
	    in[43] ^ in[44] ^ in[45] ^ in[47];
	c[23] =
	    q[0] ^ q[1] ^ q[3] ^ q[4] ^ q[10] ^ q[11] ^ q[13] ^ q[15] ^ q[18] ^
	    q[19] ^ q[20] ^ q[22] ^ q[23] ^ q[26] ^ q[30] ^ q[31] ^ in[0] ^
	    in[1] ^ in[6] ^ in[9] ^ in[13] ^ in[15] ^ in[16] ^ in[17] ^ in[19] ^
	    in[20] ^ in[26] ^ in[27] ^ in[29] ^ in[31] ^ in[34] ^ in[35] ^
	    in[36] ^ in[38] ^ in[39] ^ in[42] ^ in[46] ^ in[47];
	c[24] =
	    q[0] ^ q[1] ^ q[2] ^ q[4] ^ q[5] ^ q[11] ^ q[12] ^ q[14] ^ q[16] ^
	    q[19] ^ q[20] ^ q[21] ^ q[23] ^ q[24] ^ q[27] ^ q[31] ^ in[1] ^
	    in[2] ^ in[7] ^ in[10] ^ in[14] ^ in[16] ^ in[17] ^ in[18] ^ in[20]
	    ^ in[21] ^ in[27] ^ in[28] ^ in[30] ^ in[32] ^ in[35] ^ in[36] ^
	    in[37] ^ in[39] ^ in[40] ^ in[43] ^ in[47];
	c[25] =
	    q[1] ^ q[2] ^ q[3] ^ q[5] ^ q[6] ^ q[12] ^ q[13] ^ q[15] ^ q[17] ^
	    q[20] ^ q[21] ^ q[22] ^ q[24] ^ q[25] ^ q[28] ^ in[2] ^ in[3] ^
	    in[8] ^ in[11] ^ in[15] ^ in[17] ^ in[18] ^ in[19] ^ in[21] ^ in[22]
	    ^ in[28] ^ in[29] ^ in[31] ^ in[33] ^ in[36] ^ in[37] ^ in[38] ^
	    in[40] ^ in[41] ^ in[44];
	c[26] =
	    q[2] ^ q[3] ^ q[4] ^ q[6] ^ q[7] ^ q[8] ^ q[9] ^ q[10] ^ q[12] ^
	    q[15] ^ q[22] ^ q[23] ^ q[25] ^ q[26] ^ q[28] ^ q[31] ^ in[0] ^
	    in[3] ^ in[4] ^ in[6] ^ in[10] ^ in[18] ^ in[19] ^ in[20] ^ in[22] ^
	    in[23] ^ in[24] ^ in[25] ^ in[26] ^ in[28] ^ in[31] ^ in[38] ^
	    in[39] ^ in[41] ^ in[42] ^ in[44] ^ in[47];
	c[27] =
	    q[3] ^ q[4] ^ q[5] ^ q[7] ^ q[8] ^ q[9] ^ q[10] ^ q[11] ^ q[13] ^
	    q[16] ^ q[23] ^ q[24] ^ q[26] ^ q[27] ^ q[29] ^ in[1] ^ in[4] ^
	    in[5] ^ in[7] ^ in[11] ^ in[19] ^ in[20] ^ in[21] ^ in[23] ^ in[24]
	    ^ in[25] ^ in[26] ^ in[27] ^ in[29] ^ in[32] ^ in[39] ^ in[40] ^
	    in[42] ^ in[43] ^ in[45];
	c[28] =
	    q[4] ^ q[5] ^ q[6] ^ q[8] ^ q[9] ^ q[10] ^ q[11] ^ q[12] ^ q[14] ^
	    q[17] ^ q[24] ^ q[25] ^ q[27] ^ q[28] ^ q[30] ^ in[2] ^ in[5] ^
	    in[6] ^ in[8] ^ in[12] ^ in[20] ^ in[21] ^ in[22] ^ in[24] ^ in[25]
	    ^ in[26] ^ in[27] ^ in[28] ^ in[30] ^ in[33] ^ in[40] ^ in[41] ^
	    in[43] ^ in[44] ^ in[46];
	c[29] =
	    q[5] ^ q[6] ^ q[7] ^ q[9] ^ q[10] ^ q[11] ^ q[12] ^ q[13] ^ q[15] ^
	    q[18] ^ q[25] ^ q[26] ^ q[28] ^ q[29] ^ q[31] ^ in[3] ^ in[6] ^
	    in[7] ^ in[9] ^ in[13] ^ in[21] ^ in[22] ^ in[23] ^ in[25] ^ in[26]
	    ^ in[27] ^ in[28] ^ in[29] ^ in[31] ^ in[34] ^ in[41] ^ in[42] ^
	    in[44] ^ in[45] ^ in[47];
	c[30] =
	    q[6] ^ q[7] ^ q[8] ^ q[10] ^ q[11] ^ q[12] ^ q[13] ^ q[14] ^ q[16] ^
	    q[19] ^ q[26] ^ q[27] ^ q[29] ^ q[30] ^ in[4] ^ in[7] ^ in[8] ^
	    in[10] ^ in[14] ^ in[22] ^ in[23] ^ in[24] ^ in[26] ^ in[27] ^
	    in[28] ^ in[29] ^ in[30] ^ in[32] ^ in[35] ^ in[42] ^ in[43] ^
	    in[45] ^ in[46];
	c[31] =
	    q[7] ^ q[8] ^ q[9] ^ q[11] ^ q[12] ^ q[13] ^ q[14] ^ q[15] ^ q[17] ^
	    q[20] ^ q[27] ^ q[28] ^ q[30] ^ q[31] ^ in[5] ^ in[8] ^ in[9] ^
	    in[11] ^ in[15] ^ in[23] ^ in[24] ^ in[25] ^ in[27] ^ in[28] ^
	    in[29] ^ in[30] ^ in[31] ^ in[33] ^ in[36] ^ in[43] ^ in[44] ^
	    in[46] ^ in[47];

	for (i = 0; i < 32; i++)
	crc |= c[i] << i;

        return crc;
}


static uint32_t hash_func(const uint8_t *key, uint32_t seed, uint32_t mux_seed)
{
        const uint32_t out_width = log(TABLE_SIZE) / log(2);        /* 9bits */
//        const uint32_t out_width = 9;        /* 9bits */
        uint64_t crc_total;
        uint64_t crc_high;
        uint64_t crc_low;
        uint64_t crc_out;
        uint64_t crc32;
        uint32_t pos;

        crc32 = crc32_hash(key, seed);

        crc_total = (crc32 << 32) >> mux_seed;
        crc_high = (crc_total & 0xFFFFFFFF00000000) >> 32;
        crc_low = crc_total & 0xFFFFFFFF;

        crc_out = crc_high | crc_low;
        pos = (int)(crc_out % (1 << out_width));

        return pos;
}


#define MAX_RULE_NUM	TABLE_SIZE * BUCKET_COUNT
typedef struct {
	Rule rule[MAX_RULE_NUM];
	int rule_num;
} RuleSet;

CuckooHashTable table;

uint32_t crc32_table[256];

static void generate_crc32_table()
{
	uint32_t polynomial = 0xEDB88320;
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t c = i;
		for (size_t j = 0; j < 8; j++) {
			c = (c & 1) ? (polynomial ^ (c >> 1)) : (c >> 1);
		}
		crc32_table[i] = c;
	}
}

static uint32_t crc32(uint32_t crc, const uint8_t *buf, size_t size)
{
	crc = ~crc;
	while (size--) {
		crc = crc32_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
	}
	return ~crc;
}

static uint32_t hash1(uint64_t key)
{
	return crc32(0, (uint8_t *) & key, sizeof(key)) % TABLE_SIZE;
}

static uint32_t hash2(uint64_t key)
{
	return (crc32(0, (uint8_t *) & key, sizeof(key)) + 1) % TABLE_SIZE;
}


static void string_to_hex_array(const char *hex_str, uint8_t *key) {
    char temp[3]; // 用于存储每两个字符

    // 确保字符串长度正确
    if (strlen(hex_str) != 12) {
        printf("Invalid hexadecimal string length %d, %s.\n", strlen(hex_str), hex_str);
        return;
    }

    // 将十六进制字符串转换为字节数组
    for (int i = 0; i < 6; i++) {
        temp[0] = hex_str[i*2];
        temp[1] = hex_str[i*2 + 1];
        temp[2] = '\0'; // 确保字符串以 '\0' 结尾
        key[5 - i] = (uint8_t)strtoul(temp, NULL, 16);
    }

#if 0
    // 打印每个字节的值
    for (int i = 0; i < 6; ++i) {
        printf("0x%02x ", key[i]);
    }
    printf("\n");
#endif
}

static void parse_trace_file(const char *filename, RuleSet *rs)
{
	FILE *file = fopen(filename, "r");
	if (!file) {
		perror("Failed to open file");
		return;
	}

	char line[256];
	int num = 0;
	while (fgets(line, sizeof(line), file)) {
#if 0
		if (strncmp(line, "rule_num=", 8) == 0) {
			rs->rule_num = atoi(line + 8);
		} else
#endif
//		printf("%s\n", line);
		size_t len = strcspn(line, "\n");
		line[len] = '\0'; // 将换行符替换为字符串结束符
		if (strncmp(line, "key=", 4) == 0) {
			uint8_t key[6];
			uint8_t value[2];
			uint16_t value_tmp;

			string_to_hex_array(line + 6, key);

			if (fgets(line, sizeof(line), file)
			    && strncmp(line, "value=", 6) == 0) {
				value_tmp = (uint16_t) strtoul(line + 6, NULL, 16);
				memcpy(value, &value_tmp, sizeof(value_tmp));
			}
			memcpy(rs->rule[num].key, key, 6);
			memcpy(rs->rule[num].value, value, 2);
			num++;
		}
	}
	rs->rule_num = num;

	fclose(file);
}

static void cuckoo_key_hex2_str(uint8_t *key, char *key_str)
{
	int i;

	sprintf(key_str, "%02x%02x%02x%02x%02x%02x", key[5], key[4], key[3], key[2], key[1], key[0]);
}

static void print_key(uint8_t *key)
{
	int i;
	for (i = 0; i < 6; i++)
		printf("%02x", key[i]);

	printf("\n");
}

static int insert(uint8_t *key, struct stream *kick)
{
	int i;
	int pos[BUCKET_COUNT];
	int num = kick->count % BUCKET_COUNT;

	for (i = 0; i < BUCKET_COUNT; i++) {
		pos[i] = hash_func(key, table.seed[i], table.mux_seed[i]);
		char key_str[256];
		cuckoo_key_hex2_str(key, key_str);

		printf("key %s pos%d: %d\n", key_str, i, pos[i]);

		if (!table.buckets[i][pos[i]].occupied) {
			memcpy(table.buckets[i][pos[i]].key, key, 6);
			table.buckets[i][pos[i]].occupied = 1;
			printf("Inserted key %s at bucket %d pos %d\n", key_str, i, pos[i]);
			return 0;
		}
	}


#if 0
	// Kick out the first occupied entry
	uint8_t *kicked_key = table.buckets[0][pos[0]].key;
	memcpy(&table.buckets[0][pos[0]], &table.buckets[1][pos[1]],
	       sizeof(Bucket));
	memcpy(table.buckets[1][pos[1]].key, key, 6);
	table.buckets[1][pos[1]].occupied = 1;
#else
	//backup bucket0
	uint8_t kicked_key[6];
	memcpy(kicked_key, table.buckets[num][pos[num]].key, 6);
	//Insert key to bucket0
	memcpy(table.buckets[num][pos[num]].key, key, 6);
	table.buckets[num][pos[num]].occupied = 1;
#if 1
	for (i = 0; i < kick->count; i++) {
		if (memcmp(kick->kick[i], kicked_key, 6) == 0) {
			printf("Inserted key ERR\n");
			print_key(key);
			return -1;
		} else
			print_key(key);
	}
#endif
	memcpy(kick->kick[kick->count], kicked_key, 6);
	kick->count++;
	//insert backup key to bucket1
#endif
	// Try to insert the kicked out key
	insert(kicked_key, kick);

}

void write_buckets_to_file(const char *filename)
{
	FILE *file = fopen(filename, "w");
	if (!file) {
		perror("Failed to open file");
		return;
	}

	for (int i = 0; i < BUCKET_COUNT; i++) {
		for (int j = 0; j < TABLE_SIZE; j++) {
			if (table.buckets[i][j].occupied) {
				fprintf(file, "key=0x%llx\n",
					table.buckets[i][j].key);
			}
		}
	}

	fclose(file);
}

static void cuckoo_table_init(void)
{
        uint32_t seed[BUCKET_COUNT];
        uint32_t mux_seed[BUCKET_COUNT];
        uint32_t seed_mask;
        uint32_t mux_seed_mask;
        int no_duplicate = 0;
        int i, j;

        mux_seed_mask = (1ULL << 5) - 1;
        seed_mask = (1ULL << 32) - 1;

	srand(time(NULL) + rand());
	for (i = 0; i < BUCKET_COUNT; i++)
		seed[i] = rand() & seed_mask;

	srand(time(NULL) + rand());
	for (i = 0; i < BUCKET_COUNT; i++)
		mux_seed[i] = rand() & mux_seed_mask;


	memcpy(table.seed, seed, sizeof(uint32_t) * BUCKET_COUNT);
	memcpy(table.mux_seed, mux_seed, sizeof(uint32_t) * BUCKET_COUNT);
	printf("seed[0] %x\n", table.seed[0]);
	printf("seed[1] %x\n", table.seed[1]);
	printf("mux_seed[0] %x\n", table.mux_seed[0]);
	printf("mux_seed[1] %x\n", table.mux_seed[1]);
}

int main()
{
	int i = 0;
	RuleSet rs = { 0 };
	int ret;

	parse_trace_file("trace.txt", &rs);

	struct stream kick;
	kick.count = 0;
	cuckoo_table_init();
	for (i = 0; i < rs.rule_num; i++) {
		ret = insert(rs.rule[i].key, &kick);
		if (ret < 0)
			return ret;

	}
	write_buckets_to_file("hw.txt");
	return 0;
}
