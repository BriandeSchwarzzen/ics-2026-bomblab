#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t lo = (uint32_t)challenge;
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t s1 = (mid + 0x76acU) ^ hi;
    uint32_t s0 = (lo + hi * 8U) ^ 0x2553U;
    values[0] = (s1 ^ 0xd101U) + s0 * 8U;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    values[0] = (uint32_t)((challenge >> 14) & 0xFF) * 3U;
    for (uint32_t i = 0; i < 5; ++i) {
        uint32_t part = (uint32_t)(challenge >> (i + 3)) & 0x3FU;
        values[i + 1] = ((values[i] * 2U) ^ part) + (i + 1) * 13U;
    }
    return write_answer(out, cap, values, count);
}

static uint32_t p3_case(uint32_t idx, uint32_t x) {
    switch (idx) {
    case 0: return x * 5U + 0x3ec92U;
    case 1: return (x * 4U) ^ 0x4d2cU;
    case 2: return (x ^ 0xc11dU) + (x >> 1);
    case 3: return (x * 2U) ^ 0x62e9U;
    case 4: return x * 5U + 0xeedeU;
    case 5: return (x ^ 0xa38U) + (x >> 2);
    case 6: return x * 3U + 0xb6f1U;
    default: return x * 5U + 0x4d166U;
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t idx = (uint32_t)challenge & 7U;
    uint32_t x = ((uint32_t)(challenge >> 32)) ^ ((uint32_t)(challenge >> 3));
    values[0] = idx;
    values[1] = p3_case(idx, x);
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t target, uint32_t lo, uint32_t hi, uint32_t key) {
    uint32_t mid = lo + (hi - lo) / 2U;
    if (mid == target) return (mid + 0xdfU) ^ key;
    uint32_t node = mid + key;
    uint32_t r;
    if (target < mid) {
        r = p4_walk(target, lo, mid - 1U, key);
        return ((r * 5U) ^ node) ^ 0x3U;
    } else {
        r = p4_walk(target, mid + 1U, hi, key);
        return ((r * 5U) ^ node) ^ 0xbU;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t target = (uint32_t)challenge & 0x7fU;
    uint32_t key = ((uint32_t)(challenge >> 32)) ^ ((uint32_t)(challenge >> 7));
    values[0] = p4_walk(target, 0U, 0x7fU, key);
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    /* 按二进制 .rodata 中 p5_nodes 的地址顺序重建链表 */
    static const int32_t next_idx[12] = {1, 2, -1, 4, 0, 9, 5, 6, 7, 11, 3, 10};
    static const uint32_t f1[12] = {0x83a, 0x948, 0x763, 0xb55, 0xa6b, 0x5d, 0x575, 0x164, 0x464, 0x357, 0x626, 0x24e};
    static const uint32_t f2[12] = {0xe08c, 0x57c6, 0x426e, 0xce18, 0x8168, 0xc60, 0xd901, 0x587d, 0x2b33, 0x292, 0xf77b, 0xba15};
    static const int32_t heads[8] = {8, 7, 6, 5, 9, 11, 10, 3};
    uint32_t idx = (uint32_t)challenge & 7U;
    uint32_t n = ((uint32_t)(challenge >> 3)) & 3U;
    uint32_t x = (((uint32_t)(challenge >> 32)) ^ ((uint32_t)(challenge >> 6))) ^ 0x8e47U;
    int32_t cur = heads[idx];
    uint32_t acc = (x + x * 2U) + f2[cur];
    acc ^= f1[cur];
    uint32_t last = f1[cur];
    for (uint32_t i = 0; i < n; ++i) {
        cur = next_idx[cur];
        acc = (acc + acc * 2U) + f2[cur];
        acc ^= f1[cur];
        last = f1[cur];
    }
    values[0] = acc;
    values[1] = last;
    return write_answer(out, cap, values, count);
}

static uint32_t p6_op_0(uint32_t x) { return (x ^ (x >> 2)) + 0xd367U; }
static uint32_t p6_op_1(uint32_t x) { return ((x << 5) + 0x67f6U) ^ (x * 5U); }
static uint32_t p6_op_2(uint32_t x) { return ((x << 2) | (x >> 30)) ^ 0x19deU; }
static uint32_t p6_op_3(uint32_t x) { return (x >> 1) | (x << 31); }
static uint32_t p6_op_4(uint32_t x) { return (x + 0xd2eaU) ^ (x >> 2); }
static uint32_t p6_op_5(uint32_t x) { return (x ^ 0x600eU) + x * 4U; }

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    static uint32_t (*const ops[8])(uint32_t) = {
        p6_op_2, p6_op_4, p6_op_5, p6_op_0, p6_op_0, p6_op_2, p6_op_3, p6_op_1};
    uint32_t x = ((uint32_t)(challenge >> 6)) ^ ((uint32_t)(challenge >> 32));
    uint32_t v = ops[challenge & 7U](x) ^ 0x4c6U;
    values[0] = ops[(challenge >> 3) & 7U](v) + 0x1a57U;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t a_lo = (uint32_t)challenge;
    uint32_t a_hi = (uint32_t)(challenge >> 32);
    uint32_t b_lo = (uint32_t)context;
    uint32_t b_hi = (uint32_t)(context >> 32);
    uint32_t A = a_hi ^ a_lo;
    uint32_t B = b_hi ^ b_lo;
    uint32_t c = b_lo & 3U;
    uint32_t mix = ((A ^ B) >> 3) | ((A ^ B) << 29);  /* ror 3 */
    mix ^= 0x1e46U;
    uint32_t f2 = mix ^ ((c * 0x968U) ^ (uint32_t)(context >> 16));
    uint32_t f3 = f2 ^ (B + c + 0x172eU + (A >> 3));
    /* 第 7 行：前 32 字节随意（不校验），随后 4 字节必须是 OPEN，再跟 3 个整数 */
    static const char filler[33] = "SKY@STATION: LIGHT ON 2026-SEP!!";
    int size = snprintf(out, cap, "%sOPEN %" PRIu32 " %" PRIu32 " %" PRIu32, filler, c, f2, f3);
    if (size < 0 || (size_t)size >= cap) return 1;
    return 0;
}
