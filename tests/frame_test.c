#include "test.h"
#include "frame.h"
#include <stdlib.h>
#include <string.h>

/* frame_new: no params */

int test_frame_new_no_params() {
  frame_t *f = frame_new("f", NULL, 0);
  ASSERT(f != NULL);
  ASSERT_STR_EQ(f->name, "f");
  ASSERT_EQ(f->num_formals, 0);
  ASSERT_EQ(f->local_count, 0);
  return 1;
}
REGISTER_TEST(test_frame_new_no_params);

/* frame_new: all params escaped → all ACCESS_FRAME */

int test_frame_new_all_escaped() {
  int escapes[] = { 1, 1, 1 };
  frame_t *f = frame_new("f", escapes, 3);
  ASSERT(f != NULL);
  ASSERT_EQ(f->num_formals, 3);
  ASSERT_EQ(f->local_count, 3);
  ASSERT_EQ(f->formals[0].kind,   ACCESS_FRAME);
  ASSERT_EQ(f->formals[0].offset, -8);
  ASSERT_EQ(f->formals[1].kind,   ACCESS_FRAME);
  ASSERT_EQ(f->formals[1].offset, -16);
  ASSERT_EQ(f->formals[2].kind,   ACCESS_FRAME);
  ASSERT_EQ(f->formals[2].offset, -24);
  return 1;
}
REGISTER_TEST(test_frame_new_all_escaped);

/* frame_new: no params escaped → all ACCESS_REG with sequential indices */

int test_frame_new_none_escaped() {
  int escapes[] = { 0, 0, 0 };
  frame_t *f = frame_new("f", escapes, 3);
  ASSERT(f != NULL);
  ASSERT_EQ(f->num_formals, 3);
  ASSERT_EQ(f->local_count, 0);
  ASSERT_EQ(f->formals[0].kind, ACCESS_REG);
  ASSERT_EQ(f->formals[0].reg,  0);
  ASSERT_EQ(f->formals[1].kind, ACCESS_REG);
  ASSERT_EQ(f->formals[1].reg,  1);
  ASSERT_EQ(f->formals[2].kind, ACCESS_REG);
  ASSERT_EQ(f->formals[2].reg,  2);
  return 1;
}
REGISTER_TEST(test_frame_new_none_escaped);

/* frame_new: mixed escapes */

int test_frame_new_mixed_escapes() {
  int escapes[] = { 0, 1, 0, 1 };
  frame_t *f = frame_new("f", escapes, 4);
  ASSERT(f != NULL);
  ASSERT_EQ(f->local_count, 2);
  ASSERT_EQ(f->formals[0].kind, ACCESS_REG);
  ASSERT_EQ(f->formals[0].reg,  0);
  ASSERT_EQ(f->formals[1].kind,   ACCESS_FRAME);
  ASSERT_EQ(f->formals[1].offset, -8);
  ASSERT_EQ(f->formals[2].kind, ACCESS_REG);
  ASSERT_EQ(f->formals[2].reg,  1);
  ASSERT_EQ(f->formals[3].kind,   ACCESS_FRAME);
  ASSERT_EQ(f->formals[3].offset, -16);
  return 1;
}
REGISTER_TEST(test_frame_new_mixed_escapes);

/* frame_alloc_local: escaped local gets next frame slot */

int test_frame_alloc_local_escaped() {
  int escapes[] = { 1 };
  frame_t *f = frame_new("f", escapes, 1);
  /* one escaped param already at -8, local_count=1 */
  access_t *a = frame_alloc_local(f, 1);
  ASSERT(a != NULL);
  ASSERT_EQ(a->kind,   ACCESS_FRAME);
  ASSERT_EQ(a->offset, -16);
  ASSERT_EQ(f->local_count, 2);
  return 1;
}
REGISTER_TEST(test_frame_alloc_local_escaped);

/* frame_alloc_local: non-escaped local gets REG_TBD */

int test_frame_alloc_local_not_escaped() {
  frame_t *f = frame_new("f", NULL, 0);
  access_t *a = frame_alloc_local(f, 0);
  ASSERT(a != NULL);
  ASSERT_EQ(a->kind, ACCESS_REG);
  ASSERT_EQ(a->reg,  REG_TBD);
  ASSERT_EQ(f->local_count, 0);
  return 1;
}
REGISTER_TEST(test_frame_alloc_local_not_escaped);

/* frame_alloc_local: offsets don't collide across multiple locals */

int test_frame_alloc_local_multiple() {
  frame_t *f = frame_new("f", NULL, 0);
  access_t *a1 = frame_alloc_local(f, 1);
  access_t *a2 = frame_alloc_local(f, 1);
  access_t *a3 = frame_alloc_local(f, 1);
  ASSERT_EQ(a1->offset, -8);
  ASSERT_EQ(a2->offset, -16);
  ASSERT_EQ(a3->offset, -24);
  ASSERT_EQ(f->local_count, 3);
  return 1;
}
REGISTER_TEST(test_frame_alloc_local_multiple);

/* frame_alloc_local: locals continue after escaped params */

int test_frame_alloc_local_after_escaped_params() {
  int escapes[] = { 1, 1 };
  frame_t *f = frame_new("f", escapes, 2);
  /* params at -8, -16; local_count=2 */
  access_t *a = frame_alloc_local(f, 1);
  ASSERT_EQ(a->kind,   ACCESS_FRAME);
  ASSERT_EQ(a->offset, -24);
  return 1;
}
REGISTER_TEST(test_frame_alloc_local_after_escaped_params);
