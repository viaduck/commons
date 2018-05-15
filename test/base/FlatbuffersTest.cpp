/*
 * Copyright (C) 2018 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FlatbuffersTest.h"

TEST_F(FlatbuffersTest, testExample) {
    // put some test data in
    flatbuffers::FlatBufferBuilder builder;

    // test values
    std::string t_name("Hans Wurst"), t_num1("12982"), t_num2("12989101");
    short t_id = 0x1337;

    // generate person
    auto p_name = builder.CreateString(t_name);
    auto p_num1 = builder.CreateString(t_num1);
    auto p_num2 = builder.CreateString(t_num2);
    auto p_phone1 = tutorial::CreatePhoneNumber(builder, p_num1);
    auto p_phone2 = tutorial::CreatePhoneNumber(builder, p_num2);
    std::vector<flatbuffers::Offset<tutorial::PhoneNumber>> t_phones = {p_phone1, p_phone2};
    auto p_phones = builder.CreateVector(t_phones);

    auto person1 = tutorial::CreatePerson(builder, p_name, t_id, 0, p_phones);
    builder.Finish(person1);

    // serialize to buffer
    Buffer data;
    data.write(builder.GetBufferPointer(), builder.GetSize(), 0);

    // deserialize to person2
    auto person2 = tutorial::GetPerson(data.const_data());
    //tutorial::VerifyPersonBuffer(flatbuffers::Verifier())

    // check values
    EXPECT_EQ(t_name, person2->name()->str());
    EXPECT_EQ(t_id, person2->id());
    EXPECT_EQ(2, person2->phones()->size());
    EXPECT_EQ(t_num1, person2->phones()->Get(0)->number()->str());
    EXPECT_EQ(t_num2, person2->phones()->Get(1)->number()->str());
}