// Copyright (c) 2014, Cloudera, inc.

#include "util/sync_point.h"

#include <gtest/gtest.h>

#include "gutil/ref_counted.h"
#include "util/test_util.h"
#include "util/thread.h"

using std::string;
using std::vector;

#ifndef NDEBUG
namespace kudu {

static void RunThread(bool *var) {
  *var = true;
  TEST_SYNC_POINT("first");
}

TEST(SyncPointTest, TestSyncPoint) {
  // Set up a sync point "second" that depends on "first".
  vector<SyncPoint::Dependency> dependencies;
  dependencies.push_back(SyncPoint::Dependency("first", "second"));
  SyncPoint::GetInstance()->LoadDependency(dependencies);
  SyncPoint::GetInstance()->EnableProcessing();

  // Kick off a thread that'll process "first", but not before
  // setting 'var' to true, which unblocks the main thread.
  scoped_refptr<Thread> thread;
  bool var = false;
  ASSERT_STATUS_OK(kudu::Thread::Create("test", "test",
                                        &RunThread, &var, &thread));

  // Blocked on RunThread to process "first".
  TEST_SYNC_POINT("second");
  ASSERT_TRUE(var);

  thread->Join();
}

} // namespace kudu
#endif // NDEBUG
