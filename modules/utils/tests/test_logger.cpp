#include "openev/utils/logger.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

class CoutRedirect {
public:
  CoutRedirect(std::ostream &new_stream) : old(std::cout.rdbuf(new_stream.rdbuf())) {}
  ~CoutRedirect() { std::cout.rdbuf(old); }

private:
  std::streambuf *old;
};

TEST(LoggerTests, InfoLogging) {
  std::stringstream buffer;
  CoutRedirect redirect(buffer);
  ev::logger::info("This is an info message");
  EXPECT_EQ(buffer.str(), "INFO. openev: This is an info message\n");
}

TEST(LoggerTests, WarningLogging) {
  std::stringstream buffer;
  CoutRedirect redirect(buffer);
  ev::logger::warning("This is a warning message", false);
  EXPECT_EQ(buffer.str(), "WARNING. openev: This is a warning message\n");
}

TEST(LoggerTests, WarningLoggingWithAssertion) {
  std::stringstream buffer;
  CoutRedirect redirect(buffer);
  ev::logger::warning("This message should not appear", true);
  EXPECT_EQ(buffer.str(), "");
}

TEST(LoggerTests, ErrorLogging) {
  EXPECT_THROW(ev::logger::error("This is an error message", false), std::runtime_error);
}

TEST(LoggerTests, ErrorLoggingWithAssertion) {
  EXPECT_NO_THROW(ev::logger::error("This message should not throw", true));
}
