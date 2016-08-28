#ifndef STDREDIRECTOR_HPP
#define STDREDIRECTOR_HPP

#include <iostream>

template <class Elem = char, class Tr = std::char_traits<Elem> >

class StdRedirector : public std::basic_streambuf<Elem, Tr> {
    typedef void (*pfncb)(const Elem*, std::streamsize _Count, void* pUsrData);

public:
    StdRedirector(std::ostream& a_Stream, pfncb a_Cb, void* a_pUsrData)
        : m_Stream(a_Stream)
        , m_pCbFunc(a_Cb)
        , m_pUserData(a_pUsrData)
    {
        m_pBuf = m_Stream.rdbuf(this);
    };

    ~StdRedirector()
    {
        m_Stream.rdbuf(m_pBuf);
    }

    std::streamsize xsputn(const Elem* _Ptr, std::streamsize _Count)
    {
        m_pCbFunc(_Ptr, _Count, m_pUserData);
        return _Count;
    }

    typename Tr::int_type overflow(typename Tr::int_type v)
    {
        Elem ch[2];
        ch[0] = Tr::to_char_type(v);
        ch[1] = '\0';
        m_pCbFunc(&ch[0], 1, m_pUserData);
        return Tr::not_eof(v);
    }

protected:
    std::basic_ostream<Elem, Tr>& m_Stream;
    std::streambuf* m_pBuf;
    pfncb m_pCbFunc;
    void* m_pUserData;
};

#endif
