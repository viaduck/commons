#ifndef COMMONS_CONSTEXPRSTRING_H
#define COMMONS_CONSTEXPRSTRING_H

using size_type = std::size_t;
template <size_type Size>
class ConstexprString {
public:
    using Type = char const (&) [Size+1];

    explicit constexpr ConstexprString(Type src) : ConstexprString(src, std::make_index_sequence<Size>()){}

    constexpr size_type size() const {
        return Size;
    }

    constexpr Type c_str() const {
        return data;
    }

    template <size_type OtherSize>
    constexpr ConstexprString<Size+OtherSize> operator+(ConstexprString<OtherSize> other) const {
        return combine(*this, std::make_index_sequence<Size>(), other, std::make_index_sequence<OtherSize>());
    }

    template <size_type OtherSize>
    constexpr bool operator==(ConstexprString<OtherSize> other) const {
        return size() == other.size();
    }
    template <size_type OtherSize>
    constexpr bool operator!=(ConstexprString<OtherSize> other) const {
        return !operator==(other);
    }

private:
    template <size_type ... Indices>
    constexpr ConstexprString(Type src, std::index_sequence<Indices...>) : data{src[Indices]..., '\0'} {}

    template <size_type ... Indices2, size_type ... Indices1, size_type OtherSize>
    constexpr ConstexprString<Size+OtherSize> combine(ConstexprString<Size> src1, std::index_sequence<Indices1...>, ConstexprString<OtherSize> src2, std::index_sequence<Indices2...>) const {
        constexpr size_type newSize = Size+OtherSize;
        const char newData[newSize+1] = {src1.data[Indices1]..., src2.data[Indices2]..., '\0'};
        return ConstexprString<newSize>(newData);
    }

public:
    char data[Size+1];      // must contain \0 terminator
};

template <size_type Size>
constexpr ConstexprString<Size-1> MakeConstexprString(char const (&src) [Size]) {
    return ConstexprString<Size-1>(src);
}

#endif //COMMONS_CONSTEXPRSTRING_H
