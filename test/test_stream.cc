/* test_stream.cc - Test basic streaming functionality
   part of the minizip-ng project

   Copyright (C) Nathan Moinvaziri
     https://github.com/zlib-ng/minizip-ng

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include "mz.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"
#include "mz_strm_mem.h"

#include <gtest/gtest.h>

typedef void (*stream_test_cb)(const char *name, int32_t count, const uint8_t *find, int32_t find_size,
                               mz_stream_find_cb find_cb);

static void test_stream_find_begin(const char *name, int32_t count, const uint8_t *find, int32_t find_size,
                                   mz_stream_find_cb find_cb) {
    void *mem_stream = NULL;
    int32_t i = 0;
    int32_t x = 0;
    int64_t last_pos = 0;
    int64_t position = 0;

    MZ_UNUSED(name);

    ASSERT_GT(find_size, 0);
    ASSERT_NE(find, nullptr);
    ASSERT_NE(find_cb, nullptr);

    for (i = 0; i < count; i++) {
        mem_stream = mz_stream_mem_create();
        ASSERT_NE(mem_stream, nullptr);
        mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE);

        /* Find when the needle is at the beginning of the stream */
        for (x = 0; x < i && x < find_size; x++)
            mz_stream_write_uint8(mem_stream, find[x]);
        while (x++ < i)
            mz_stream_write_uint8(mem_stream, 0);

        if (find_cb == mz_stream_find)
            mz_stream_seek(mem_stream, 0, MZ_SEEK_SET);

        find_cb(mem_stream, (const void *)find, find_size, i, &position);

        /* Should always find at the start of the stream if entire needle
           was written to stream */
        EXPECT_EQ(position, (i < find_size) ? -1 : 0) << "name: " << name << std::endl
                                                      << "find_size: " << find_size << std::endl
                                                      << "index: " << i << std::endl;

        mz_stream_seek(mem_stream, 0, MZ_SEEK_END);
        last_pos = mz_stream_tell(mem_stream);
        mz_stream_mem_delete(&mem_stream);

        /* Shouldn't be at the end of the stream */
        EXPECT_NE(position, last_pos);
    }
}

static void test_stream_find_end(const char *name, int32_t count, const uint8_t *find, int32_t find_size,
                                 mz_stream_find_cb find_cb) {
    void *mem_stream = NULL;
    int32_t i = 0;
    int32_t x = 0;
    int32_t y = 0;
    int64_t last_pos = 0;
    int64_t position = 0;

    MZ_UNUSED(name);

    ASSERT_GT(find_size, 0);
    ASSERT_NE(find, nullptr);
    ASSERT_NE(find_cb, nullptr);

    for (i = 0; i < count; i++) {
        mem_stream = mz_stream_mem_create();
        ASSERT_NE(mem_stream, nullptr);
        mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE);

        /* Find when the needle is at the end of the stream */
        for (x = 0; x < i - find_size; x++)
            mz_stream_write_uint8(mem_stream, 0);
        for (y = 0; x + y < i && y < find_size; y++)
            mz_stream_write_uint8(mem_stream, find[y]);

        if (find_cb == mz_stream_find)
            mz_stream_seek(mem_stream, 0, MZ_SEEK_SET);

        find_cb(mem_stream, (const void *)find, find_size, i, &position);

        /* Should always find after zeros if entire needle
           was written to stream */
        EXPECT_EQ(position, (i < find_size) ? -1 : (i - find_size)) << "name: " << name << std::endl
                                                                    << "find_size: " << find_size << std::endl
                                                                    << "index: " << i << std::endl;

        mz_stream_seek(mem_stream, 0, MZ_SEEK_END);
        last_pos = mz_stream_tell(mem_stream);
        mz_stream_mem_delete(&mem_stream);

        /* Shouldn't be at the end of the stream */
        EXPECT_NE(position, last_pos);
    }
}

static void test_stream_find_middle(const char *name, int32_t count, const uint8_t *find, int32_t find_size,
                                    mz_stream_find_cb find_cb) {
    void *mem_stream = NULL;
    int32_t i = 0;
    int32_t x = 0;
    int64_t last_pos = 0;
    int64_t position = 0;

    MZ_UNUSED(name);

    ASSERT_GT(find_size, 0);
    ASSERT_NE(find, nullptr);
    ASSERT_NE(find_cb, nullptr);

    for (i = 0; i < count; i++) {
        mem_stream = mz_stream_mem_create();
        ASSERT_NE(mem_stream, nullptr);
        mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE);

        /* Find when the neddle is in the middle of the stream */
        for (x = 0; x < i; x++)
            mz_stream_write_uint8(mem_stream, 0);
        mz_stream_write(mem_stream, find, find_size);
        for (x = 0; x < i; x++)
            mz_stream_write_uint8(mem_stream, 0);

        if (find_cb == mz_stream_find)
            mz_stream_seek(mem_stream, 0, MZ_SEEK_SET);

        find_cb(mem_stream, (const void *)find, find_size, i + find_size + i, &position);

        /* Should always find after initial set of zeros */
        EXPECT_EQ(position, i) << "name: " << name << std::endl
                               << "find_size: " << find_size << std::endl
                               << "index: " << i << std::endl;

        mz_stream_seek(mem_stream, 0, MZ_SEEK_END);
        last_pos = mz_stream_tell(mem_stream);
        mz_stream_mem_delete(&mem_stream);

        /* Shouldn't be at the end of the stream */
        EXPECT_NE(position, last_pos);
    }
}

static void test_stream_find_middle_odd(const char *name, int32_t count, const uint8_t *find, int32_t find_size,
                                        mz_stream_find_cb find_cb) {
    void *mem_stream = NULL;
    int32_t i = 0;
    int32_t x = 0;
    int64_t last_pos = 0;
    int64_t position = 0;

    MZ_UNUSED(name);

    ASSERT_GT(find_size, 0);
    ASSERT_NE(find, nullptr);
    ASSERT_NE(find_cb, nullptr);

    for (i = 0; i < count; i++) {
        mem_stream = mz_stream_mem_create();
        ASSERT_NE(mem_stream, nullptr);
        mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE);

        /* Find when the needle is in the middle of the stream */
        for (x = 0; x < i; x++)
            mz_stream_write_uint8(mem_stream, 0);
        mz_stream_write(mem_stream, find, find_size);
        for (x = 0; x < i + 1; x++)
            mz_stream_write_uint8(mem_stream, 0);

        if (find_cb == mz_stream_find)
            mz_stream_seek(mem_stream, 0, MZ_SEEK_SET);

        find_cb(mem_stream, (const void *)find, find_size, i + find_size + i + 1, &position);

        /* Should always find after initial set of zeros */
        EXPECT_EQ(position, i) << "name: " << name << std::endl
                               << "find_size: " << find_size << std::endl
                               << "index: " << i << std::endl;

        mz_stream_seek(mem_stream, 0, MZ_SEEK_END);
        last_pos = mz_stream_tell(mem_stream);
        mz_stream_mem_delete(&mem_stream);

        /* Shouldn't be at the end of the stream */
        EXPECT_NE(position, last_pos);
    }
}

struct stream_find_param {
    const char *name;
    stream_test_cb test_cb;
    mz_stream_find_cb find_cb;

    friend std::ostream &operator<<(std::ostream &os, const stream_find_param &param) {
        return os << "name: " << param.name;
    }
};

constexpr stream_find_param find_tests[] = {
    {             "begin",      test_stream_find_begin,         mz_stream_find},
    {     "begin reverse",      test_stream_find_begin, mz_stream_find_reverse},
    {               "end",        test_stream_find_end,         mz_stream_find},
    {       "end reverse",        test_stream_find_end, mz_stream_find_reverse},
    {            "middle",     test_stream_find_middle,         mz_stream_find},
    {    "middle reverse",     test_stream_find_middle, mz_stream_find_reverse},
    {        "middle odd", test_stream_find_middle_odd,         mz_stream_find},
    {"middle odd reverse", test_stream_find_middle_odd, mz_stream_find_reverse}
};

class stream_find : public ::testing::TestWithParam<stream_find_param> {};

INSTANTIATE_TEST_SUITE_P(stream, stream_find, testing::ValuesIn(find_tests));

TEST_P(stream_find, find) {
    const auto &param = GetParam();
    const char *find = "0123456789";
    int32_t c = 1;

    for (c = 1; c < (int32_t)strlen(find); c += 1)
        param.test_cb(param.name, 2096, (const uint8_t *)find, c, param.find_cb);
}

TEST(stream_buffered_seek, end_flushes_pending_write) {
    void *mem_stream = mz_stream_mem_create();
    void *buffered_stream = mz_stream_buffered_create();
    const void *buffer = NULL;
    int64_t length = 0;

    ASSERT_NE(mem_stream, nullptr);
    ASSERT_NE(buffered_stream, nullptr);
    ASSERT_EQ(mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE), MZ_OK);
    mz_stream_set_base(buffered_stream, mem_stream);

    EXPECT_EQ(mz_stream_write(buffered_stream, "abc", 3), 3);
    EXPECT_EQ(mz_stream_seek(buffered_stream, -1, MZ_SEEK_END), MZ_OK);
    EXPECT_EQ(mz_stream_write(buffered_stream, "Z", 1), 1);
    EXPECT_EQ(mz_stream_close(buffered_stream), MZ_OK);

    mz_stream_mem_get_buffer_length(mem_stream, &length);
    ASSERT_EQ(length, 3);
    ASSERT_EQ(mz_stream_mem_get_buffer(mem_stream, &buffer), MZ_OK);
    EXPECT_EQ(memcmp(buffer, "abZ", 3), 0);

    mz_stream_buffered_delete(&buffered_stream);
    mz_stream_mem_delete(&mem_stream);
}

TEST(stream_buffered_seek, cur_before_start_preserves_position) {
    char input[] = "abcdefghij";
    char output[5];
    char next = 0;
    void *mem_stream = mz_stream_mem_create();
    void *buffered_stream = mz_stream_buffered_create();

    ASSERT_NE(mem_stream, nullptr);
    ASSERT_NE(buffered_stream, nullptr);
    mz_stream_mem_set_buffer(mem_stream, input, 10);
    ASSERT_EQ(mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_READ), MZ_OK);
    mz_stream_set_base(buffered_stream, mem_stream);

    ASSERT_EQ(mz_stream_read(buffered_stream, output, sizeof(output)), 5);
    EXPECT_EQ(mz_stream_seek(buffered_stream, -6, MZ_SEEK_CUR), MZ_SEEK_ERROR);
    EXPECT_EQ(mz_stream_tell(buffered_stream), 5);
    EXPECT_EQ(mz_stream_read(buffered_stream, &next, 1), 1);
    EXPECT_EQ(next, 'f');

    mz_stream_buffered_delete(&buffered_stream);
    mz_stream_mem_delete(&mem_stream);
}

TEST(stream_buffered_seek, cur_within_read_buffer) {
    char input[] = "abcdefghij";
    char output[5];
    char next = 0;
    void *mem_stream = mz_stream_mem_create();
    void *buffered_stream = mz_stream_buffered_create();

    ASSERT_NE(mem_stream, nullptr);
    ASSERT_NE(buffered_stream, nullptr);
    mz_stream_mem_set_buffer(mem_stream, input, 10);
    ASSERT_EQ(mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_READ), MZ_OK);
    mz_stream_set_base(buffered_stream, mem_stream);

    ASSERT_EQ(mz_stream_read(buffered_stream, output, sizeof(output)), 5);
    EXPECT_EQ(mz_stream_seek(buffered_stream, -2, MZ_SEEK_CUR), MZ_OK);
    EXPECT_EQ(mz_stream_tell(buffered_stream), 3);
    EXPECT_EQ(mz_stream_read(buffered_stream, &next, 1), 1);
    EXPECT_EQ(next, 'd');

    mz_stream_buffered_delete(&buffered_stream);
    mz_stream_mem_delete(&mem_stream);
}

TEST(stream_buffered_seek, cur_beyond_write_buffer_flushes) {
    void *mem_stream = mz_stream_mem_create();
    void *buffered_stream = mz_stream_buffered_create();
    const void *buffer = NULL;
    int64_t length = 0;

    ASSERT_NE(mem_stream, nullptr);
    ASSERT_NE(buffered_stream, nullptr);
    ASSERT_EQ(mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE), MZ_OK);
    ASSERT_EQ(mz_stream_write(mem_stream, "0123456789", 10), 10);
    ASSERT_EQ(mz_stream_seek(mem_stream, 5, MZ_SEEK_SET), MZ_OK);
    mz_stream_set_base(buffered_stream, mem_stream);
    ASSERT_EQ(mz_stream_tell(buffered_stream), 5);

    EXPECT_EQ(mz_stream_write(buffered_stream, "abc", 3), 3);
    EXPECT_EQ(mz_stream_seek(buffered_stream, -1, MZ_SEEK_CUR), MZ_OK);
    EXPECT_EQ(mz_stream_seek(buffered_stream, 3, MZ_SEEK_CUR), MZ_OK);
    EXPECT_EQ(mz_stream_tell(buffered_stream), 10);
    EXPECT_EQ(mz_stream_write(buffered_stream, "Z", 1), 1);
    EXPECT_EQ(mz_stream_close(buffered_stream), MZ_OK);

    mz_stream_mem_get_buffer_length(mem_stream, &length);
    ASSERT_EQ(length, 11);
    ASSERT_EQ(mz_stream_mem_get_buffer(mem_stream, &buffer), MZ_OK);
    EXPECT_EQ(memcmp(buffer, "01234abc89Z", 11), 0);

    mz_stream_buffered_delete(&buffered_stream);
    mz_stream_mem_delete(&mem_stream);
}

struct stream_no_tell {
    mz_stream stream;
    int32_t seek_calls;
    int32_t tell_calls;
};

static int32_t stream_no_tell_is_open(void *stream) {
    return mz_stream_is_open(((stream_no_tell *)stream)->stream.base);
}

static int32_t stream_no_tell_read(void *stream, void *buf, int32_t size) {
    return mz_stream_read(((stream_no_tell *)stream)->stream.base, buf, size);
}

static int32_t stream_no_tell_write(void *stream, const void *buf, int32_t size) {
    return mz_stream_write(((stream_no_tell *)stream)->stream.base, buf, size);
}

static int64_t stream_no_tell_tell(void *stream) {
    ((stream_no_tell *)stream)->tell_calls += 1;
    return MZ_INTERNAL_ERROR;
}

static int32_t stream_no_tell_seek(void *stream, int64_t offset, int32_t origin) {
    ((stream_no_tell *)stream)->seek_calls += 1;
    return mz_stream_seek(((stream_no_tell *)stream)->stream.base, offset, origin);
}

static int32_t stream_no_tell_close(void *stream) {
    return mz_stream_close(((stream_no_tell *)stream)->stream.base);
}

static int32_t stream_no_tell_error(void *stream) {
    return mz_stream_error(((stream_no_tell *)stream)->stream.base);
}

static mz_stream_vtbl stream_no_tell_vtbl = {NULL, stream_no_tell_is_open, stream_no_tell_read,  stream_no_tell_write,
                                            stream_no_tell_tell, stream_no_tell_seek,    stream_no_tell_close,
                                            stream_no_tell_error, NULL, NULL, NULL, NULL};

TEST(stream_buffered_seek, end_without_base_tell_keeps_write_read_position) {
    void *mem_stream = mz_stream_mem_create();
    void *buffered_stream = mz_stream_buffered_create();
    stream_no_tell base = {};
    char next = 0;
    const void *buffer = NULL;

    ASSERT_NE(mem_stream, nullptr);
    ASSERT_NE(buffered_stream, nullptr);
    ASSERT_EQ(mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE), MZ_OK);
    ASSERT_EQ(mz_stream_write(mem_stream, "abcdef", 6), 6);
    base.stream.vtbl = &stream_no_tell_vtbl;
    mz_stream_set_base(&base, mem_stream);
    mz_stream_set_base(buffered_stream, &base);

    EXPECT_EQ(mz_stream_seek(buffered_stream, -3, MZ_SEEK_END), MZ_OK);
    EXPECT_EQ(base.tell_calls, 0);
    EXPECT_EQ(mz_stream_write(buffered_stream, "XYZ", 3), 3);
    EXPECT_EQ(mz_stream_tell(buffered_stream), MZ_INTERNAL_ERROR);
    EXPECT_EQ(mz_stream_seek(buffered_stream, -2, MZ_SEEK_CUR), MZ_OK);
    EXPECT_EQ(mz_stream_read(buffered_stream, &next, 1), 1);
    EXPECT_EQ(next, 'Y');
    EXPECT_EQ(mz_stream_close(buffered_stream), MZ_OK);
    ASSERT_EQ(mz_stream_mem_get_buffer(mem_stream, &buffer), MZ_OK);
    EXPECT_EQ(memcmp(buffer, "abcXYZ", 6), 0);

    mz_stream_buffered_delete(&buffered_stream);
    mz_stream_mem_delete(&mem_stream);
}

TEST(stream_buffered_seek, end_without_base_tell_skips_set_fast_path) {
    void *mem_stream = mz_stream_mem_create();
    void *buffered_stream = mz_stream_buffered_create();
    stream_no_tell base = {};
    char next = 0;
    const void *buffer = NULL;
    int32_t seek_calls = 0;

    ASSERT_NE(mem_stream, nullptr);
    ASSERT_NE(buffered_stream, nullptr);
    ASSERT_EQ(mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE), MZ_OK);
    ASSERT_EQ(mz_stream_write(mem_stream, "abcdefghijklmnopqrst", 20), 20);
    base.stream.vtbl = &stream_no_tell_vtbl;
    mz_stream_set_base(&base, mem_stream);
    mz_stream_set_base(buffered_stream, &base);

    EXPECT_EQ(mz_stream_seek(buffered_stream, 2, MZ_SEEK_SET), MZ_OK);
    EXPECT_EQ(mz_stream_seek(buffered_stream, 1, MZ_SEEK_CUR), MZ_OK);
    EXPECT_EQ(mz_stream_seek(buffered_stream, -3, MZ_SEEK_END), MZ_OK);
    EXPECT_EQ(mz_stream_seek(buffered_stream, 0, MZ_SEEK_CUR), MZ_OK);
    EXPECT_EQ(base.tell_calls, 0);
    EXPECT_EQ(mz_stream_read(buffered_stream, &next, 1), 1);
    EXPECT_EQ(next, 'r');
    EXPECT_EQ(mz_stream_write(buffered_stream, "Q", 1), 1);
    seek_calls = base.seek_calls;
    EXPECT_EQ(mz_stream_seek(buffered_stream, 3, MZ_SEEK_SET), MZ_OK);
    EXPECT_EQ(base.seek_calls, seek_calls + 1);
    EXPECT_EQ(mz_stream_read(buffered_stream, &next, 1), 1);
    EXPECT_EQ(next, 'd');
    ASSERT_EQ(mz_stream_mem_get_buffer(mem_stream, &buffer), MZ_OK);
    EXPECT_EQ(memcmp(buffer, "abcdefghijklmnopqrQt", 20), 0);

    mz_stream_buffered_delete(&buffered_stream);
    mz_stream_mem_delete(&mem_stream);
}
