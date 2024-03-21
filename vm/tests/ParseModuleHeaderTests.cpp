#include <gtest/gtest.h>
#include <iostream>
#include <vector>

#include "vm.hpp"

// Demonstrate some basic assertions.
TEST(ParseModuleHeader, CorrectHeader_ParsesFields) {
   std::vector<unsigned char> module_buf = {
      5, // module name len
      'm',  'y', 'm', 'o', 'd',
      3, // num exports
      1, // export name len
      'x',
      0xCD, // export offset LSB
      0xAB, // export offset MSB
      2,    // export name len
      'q',  'r',
      0x23, // export offset LSB
      0x01, // export offset MSB
      3,    // export name len
      'w',  'o', 'w',
      0x01, // export offset LSB
      0x00, // export offset MSB
   };

   auto result = vm::Module::load(module_buf);
   ASSERT_TRUE(result.has_value());
   auto mod = *result;

   EXPECT_EQ(mod.name(), std::string_view("mymod"));

   auto exp0 = mod.nth_export(0);
   ASSERT_TRUE(exp0.has_value());
   EXPECT_EQ(exp0->name, std::string_view("x"));
   EXPECT_EQ(exp0->bytecode_offset, 0xABCD);

   auto exp1 = mod.nth_export(1);
   ASSERT_TRUE(exp1.has_value());
   EXPECT_EQ(exp1->name, std::string_view("qr"));
   EXPECT_EQ(exp1->bytecode_offset, 0x0123);

   auto exp2 = mod.nth_export(2);
   ASSERT_TRUE(exp2.has_value());
   EXPECT_EQ(exp2->name, std::string_view("wow"));
   EXPECT_EQ(exp2->bytecode_offset, 1);
}

TEST(ParseModuleHeader, SingleByteHeader_Fails) {
   std::vector<unsigned char> module_buf = {0};

   auto result = vm::Module::load(module_buf);
   EXPECT_FALSE(result.has_value());
   EXPECT_EQ(result.error(), vm::Error::InvalidHeader);
}

TEST(ParseModuleHeader, WrongModuleNameLength_Fails) {
   std::vector<unsigned char> module_buf = {3, 'a', 'b'};

   auto result = vm::Module::load(module_buf);
   EXPECT_FALSE(result.has_value());
   EXPECT_EQ(result.error(), vm::Error::InvalidHeader);
}