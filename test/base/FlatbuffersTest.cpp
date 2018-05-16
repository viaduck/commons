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
    auto phone1 = new tutorial::PhoneNumberT();
    phone1->number = t_num1;

    auto phone2 = new tutorial::PhoneNumberT();
    phone2->number = t_num2;

    tutorial::PersonT person1;
    person1.name = t_name;
    person1.id = t_id;
    person1.phones.emplace_back(phone1);
    person1.phones.emplace_back(phone2);

    builder.Finish(tutorial::Person::Pack(builder, &person1));

    // serialize to buffer
    Buffer data;
    data.write(builder.GetBufferPointer(), builder.GetSize(), 0);

    // check buffer
    flatbuffers::Verifier verifier(static_cast<const uint8_t*>(data.const_data()), data.size());
    ASSERT_TRUE(tutorial::VerifyPersonBuffer(verifier));

    // deserialize to person2
    auto person2 = tutorial::UnPackPerson(data.const_data());

    // check values
    EXPECT_EQ(t_name, person2->name);
    EXPECT_EQ(t_id, person2->id);
    EXPECT_EQ(2, person2->phones.size());
    EXPECT_EQ(t_num1, person2->phones.at(0)->number);
    EXPECT_EQ(t_num2, person2->phones.at(1)->number);
}