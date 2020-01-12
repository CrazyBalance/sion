#pragma once
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#pragma warning(disable : 6031)
#pragma warning(disable : 26451)
#pragma warning(disable : 26444)
#include <string>
#include <map>
#include <regex>
#include <array>
#include <vector>
#include <codecvt>
#include <locale>
#include <mutex>
#include <functional>
#ifdef _WIN32
#include <WS2tcpip.h>
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")
#else _linux
#endif // WIN32
#ifndef SION_DISABLE_SSL
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#endif // !SION_DISABLE_SSL
namespace sion
{
    using std::array;
    using std::map;
    using std::pair;
    using std::string;
    using std::vector;

    class MyString : public string
    {
    public:
        MyString() = default;
        ~MyString() = default;
        template <class T>
        MyString(T&& arg) : string(std::forward<T>(arg)) {}

        // ʹ���ַ����ָ�
        // flag �ָ��־,���ص��ַ����������޳�,flag��Ҫ��char�������ز���ȷ
        // num �ָ������Ĭ��0���ָ��������num=1,���ؿ�ͷ��flag,flag������size=2���ַ�������
        // skipEmpty �������ַ���������ѹ��length==0���ַ���
        vector<MyString> Split(MyString flag, int num = 0, bool skipEmpty = true)
        {
            vector<MyString> dataSet;
            auto PushData = [&](MyString line) {
                if (line.length() != 0 || !skipEmpty)
                {
                    dataSet.push_back(line);
                }
            };
            auto Pos = FindAll(flag, num != 0 ? num : -1);
            if (Pos.size() == 0)
            {
                return { *this };
            }
            for (int i = 0; i < Pos.size() + 1; i++)
            {
                if (dataSet.size() == num && Pos.size() > num&& num != 0)
                { // ��������ֱ�ӽص�����
                    PushData(substr(Pos[dataSet.size()] + flag.size()));
                    break;
                }
                if (i == 0)
                { // ��һ������λ�ò���0�Ļ�����
                    PushData(substr(0, Pos[0]));
                }
                else if (i != Pos.size())
                {
                    int Left = Pos[i - 1] + flag.length();
                    int Right = Pos[i] - Left;
                    PushData(substr(Left, Right));
                }
                else
                { // ���һ����־������
                    PushData(substr(*(--Pos.end()) + flag.size()));
                }
            }
            return dataSet;
        }

        // ���ǰ����ַ�
        // target ��Ҫ������ַ�Ĭ�Ͽո�
        MyString Trim(MyString target = " ")
        {
            auto left = find_first_not_of(target);
            if (left == string::npos)
            {
                return *this;
            }
            auto right = find_last_not_of(target);
            return substr(left, right - left + target.length());
        }

        MyString ToLowerCase()
        {
            MyString s = *this;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
            return s;
        }

        MyString ToUpperCase()
        {
            MyString s = *this;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
            return s;
        }

        // ������ĸ
        bool HasLetter()
        {
            for (auto& x : *this)
            {
                if ((x >= 'a' && x <= 'z') ||
                    (x >= 'A' && x <= 'Z'))
                {
                    return true;
                }
            }
            return false;
        }

        // ת����gbk
        MyString ToGbk()
        {
            // ��ƽ̨����ֻ��c++14������,����api:blog.csdn.net/u012234115/article/details/83186386
            return MyString::utf8_to_gb2312(*this);
        }

        MyString ToUtf8()
        {
            return MyString::gb2312_to_utf8(*this);
        }
        // ����������������λ��
        // flag ��λ��־
        // num ����������Ĭ��ֱ������
        vector<int> FindAll(MyString flag, int num = -1)
        {
            vector<int> Result;
            auto Pos = find(flag);
            while (Pos != -1 && Result.size() != num)
            {
                Result.push_back(Pos);
                Pos = find(flag, *(--Result.end()) + flag.length());
            }
            return Result;
        }

        static std::string gb2312_to_utf8(std::string const& strGb2312)
        {
            // src:www.zhihu.com/question/61139105/answer/711597486
            std::vector<wchar_t> buff(strGb2312.size());
#ifdef _MSC_VER
            std::locale loc("zh-CN");
#else
            std::locale loc("zh_CN.GB18030");
#endif
            wchar_t* pwszNext = nullptr;
            const char* pszNext = nullptr;
            mbstate_t state = {};
            int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc).in(state,
                strGb2312.data(), strGb2312.data() + strGb2312.size(), pszNext,
                buff.data(), buff.data() + buff.size(), pwszNext);
            if (std::codecvt_base::ok == res)
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
                return cutf8.to_bytes(std::wstring(buff.data(), pwszNext));
            }
            return "";
        }

        static std::string utf8_to_gb2312(std::string const& strUtf8)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
            std::wstring wTemp = cutf8.from_bytes(strUtf8);
#ifdef _MSC_VER
            std::locale loc("zh-CN");
#else
            std::locale loc("zh_CN.GB18030");
#endif
            const wchar_t* pwszNext = nullptr;
            char* pszNext = nullptr;
            std::mbstate_t mb = std::mbstate_t();
            std::vector<char> buff(wTemp.size() * 2);
            int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc).out(mb,
                wTemp.data(), wTemp.data() + wTemp.size(), pwszNext,
                buff.data(), buff.data() + buff.size(), pszNext);
            if (std::codecvt_base::ok == res)
            {
                return std::string(buff.data(), pszNext);
            }
            return "";
        }

        // �ַ����滻
        // oldStr ���滻���ַ���
        // newStr �»��ϵ��ַ���
        // count �滻������Ĭ��1������0ʱ�滻���㹻�������Ҳ������ַ���Ϊֹ��С��0ʱ�滻������
        MyString& Replace(MyString oldStr, MyString newStr, int count = 1)
        {
            if (count == 0)
            {
                return *this;
            }
            int pos = find(oldStr);
            if (pos == string::npos)
            {
                return *this;
            }
            replace(pos, oldStr.length(), newStr);
            return Replace(oldStr, newStr, count < 0 ? -1 : count - 1);
        }
    };

    template <typename ExceptionType>
    void Throw(MyString msg = "") { throw ExceptionType(msg.c_str()); }

    template <typename ExceptionType = std::exception>
    void check(bool condition, MyString msg = "", std::function<void()> recycle = [] {})
    {
        if (!condition)
        {
            recycle();
            Throw<ExceptionType>(msg);
        }
    }
    using Socket = SOCKET;

    class Socket_
    {
    };

    class SocketFactory
    {
    public:
        static map<MyString, Socket> SocketAll;

        static Socket Get(MyString host)
        {
            std::mutex mu;
            std::lock_guard<std::mutex> lock(mu);
            return -1;
        }
    };

    MyString GetIpByHost(MyString hostname)
    {
        addrinfo hints, * res;
        in_addr addr;
        int err;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_INET; // ipv4
        if ((err = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0)
        {
            Throw<std::runtime_error>("����" + err + MyString(gai_strerror(err)));
        }
        addr.s_addr = ((sockaddr_in*)(res->ai_addr))->sin_addr.s_addr;
        char str[INET_ADDRSTRLEN];
        auto ptr = inet_ntop(AF_INET, &addr, str, sizeof(str));
        freeaddrinfo(res);
        return str;
    }

    Socket GetSocket()
    {
        // ��ʼ����,WSA windows�첽�׽���
        WSADATA inet_WsaData;                      //
        WSAStartup(MAKEWORD(2, 0), &inet_WsaData); // socket2.0�汾
        if (LOBYTE(inet_WsaData.wVersion) != 2 || HIBYTE(inet_WsaData.wVersion) != 0)
        { // ��λ�ֽ�ָ�����汾����λ�ֽ�ָ�����汾
            WSACleanup();
            return -1;
        }
        auto tcp_socket = socket(AF_INET, SOCK_STREAM, 0); // ipv4,tcp,tcp��udp�ò�����Ϊ0
        return tcp_socket;
    }

    class Header
    {
    public:
        Header() = default;
        ~Header() = default;
        vector<pair<MyString, MyString>> data;
        Header& operator=(vector<pair<MyString, MyString>>&& other) noexcept
        {
            data = other;
            return *this;
        }
        // ���һ����ֵ�Ե�ͷ��
        void Add(MyString k, MyString v)
        {
            data.push_back({ k, v });
        }
        // ��ȡͷ�ļ�����Ӧ������ֵ
        vector<MyString> GetValue(MyString key)
        {
            key = key.ToLowerCase();
            vector<MyString> res;
            for (auto& i : data)
            {
                if (i.first == key)
                {
                    res.push_back(i.second);
                }
            }
            return res;
        }
        // ��ȡͷ�ļ�����Ӧ��ֵ�������һ��Ϊ׼
        MyString GetLastValue(MyString key)
        {
            key = key.ToLowerCase();
            for (int i = data.size() - 1; i >= 0; i--)
            {
                if (data[i].first == key)
                {
                    return data[i].second;
                }
            }
            return "";
        }
    };

    enum class Method
    {
        Get,
        Post,
        Put,
        Delete
    };

    class Response
    {

    public:
        bool IsChunked = false;     // �Ƿ�ֿ����
        bool SaveByCharVec = false; // �Ƿ�ʹ���ַ����鱣�棬�����ı�ֱ�����ַ������棬������char����
        int ContentLength = 0;      // ���ĳ���
        MyString Source;            // Դ��Ӧ����
        MyString Cookie;
        MyString ProtocolVersion;
        MyString Code;
        MyString Status;
        MyString BodyStr; // ��Ӧ�壬�����ı�ֱ�ӱ�����
        MyString CharSet;
        vector<char> BodyCharVec; // ��Ӧ�壬���ڶ���������������
        Header ResponseHeader;    // ��Ӧͷ
        Response() = default;
        ~Response() = default;

        Response(MyString source) noexcept
        {
            Source = source;
            ParseFromSource();
        }
        MyString HeaderValue(MyString k) { return ResponseHeader.GetLastValue(k); };

        // �������������͹�������Ӧ
        // PreParse Ԥ���������Ϊtrue��ʹ��char������Ҳ���chunked������ôֻ����ͷ��
        void ParseFromSource(bool PreParse = false)
        {
            // ͷ������һ�ξ͹�
            if (ResponseHeader.data.size() == 0)
            {
                HeaderAndFirstLineParse();
            }
            // ��Ӧ�壬Ԥ������ʱ�������Ӧ��
            // �ر�Ԥ�����������ǷǷֿ���Ϊ�ַ�����ʱ����н�������Ϊ���������Ҫ��ȡͷ�ĳ���
            if (!PreParse || (!IsChunked && !SaveByCharVec))
            {
                BodyStrParse();
            }
        }

    private:
        void HeaderAndFirstLineParse()
        {
            auto HeaderStr = Source.substr(0, Source.find("\r\n\r\n"));
            auto data = MyString(HeaderStr).Split("\r\n");
            if (data.size() == 0)
            {
                return;
            }
            // ��һ��
            auto FirstLine = data[0].Split(" ", 2);
            check<std::runtime_error>(FirstLine.size() == 3, "��������\n" + Source);
            ProtocolVersion = FirstLine[0].Trim();
            Code = FirstLine[1].Trim();
            Status = FirstLine[2].Trim();
            data.erase(data.begin());
            // ͷ
            for (auto& x : data)
            {
                auto pair = x.Split(":", 1);
                if (pair.size() == 2)
                {
                    ResponseHeader.Add(pair[0].Trim().ToLowerCase(), pair[1].Trim());
                }
            }
            MyString contentLen = HeaderValue("content-length");
            ContentLength = contentLen != "" ? stoi(contentLen) : ContentLength;
            IsChunked = ContentLength == 0;
            Cookie = HeaderValue("cookie");
            auto ContentType = HeaderValue("content-type");
            if (ContentType != "")
            { // Content-Type: text/html; charset=utf-8
                auto ContentSplit = ContentType.Split(";", 1);
                auto type = ContentSplit[0].Split("/", 1)[0].Trim();
                SaveByCharVec = type != "text" && type != "application"; // ���������ı����ֽ���
                if (ContentSplit.size() != 1)                            // ��gbk�Ļ��Ѿ�ת��һ���ˣ������ٹ�
                {
                    CharSet = ContentSplit[1].Split("=", 1)[1].Trim();
                }
            }
        }

        void BodyStrParse()
        {
            if (SaveByCharVec)
            {
                const auto& sc = BodyCharVec;
                if (sc.size() == 0 || !IsChunked)
                {
                    return;
                }
                vector<char> PureSouceChar;
                // ��ȡ��һ��\r\n��λ��
                int NRpos = 0;
                auto GetNextNR = [&](int leap) {
                    for (int i = NRpos + leap; i < (sc.size() - 1); i++)
                    {
                        if (sc[i] == '\r' && sc[i + 1] == '\n')
                        {
                            NRpos = i;
                            return i;
                        }
                    }
                    return -1;
                };
                int Left = -2; // ����-2����Ϊ��һ��������400\r\n�����ģ�����������\r\n400\r\n������Ҫ�Ե�һ�ν��в���
                int Right = GetNextNR(0);
                while (Left != -1 && Right != -1)
                {
                    auto count = string(sc.begin() + 2 + Left, sc.begin() + Right); // ÿ���ֿ鿪ͷд������
                    if (count == "0")
                    {
                        break;
                    }                                           // ���һ�� 0\r\n\r\n���˳�
                    auto countNum = stoi(count, nullptr, 16);   // ��������16����
                    auto chunkedStart = sc.begin() + Right + 2; // ÿ���ֿ����ĵĿ�ʼλ��
                    PureSouceChar.insert(PureSouceChar.end(), chunkedStart, chunkedStart + countNum);
                    Left = GetNextNR(countNum); //  ����λ��
                    Right = GetNextNR(1);
                }
                BodyCharVec = PureSouceChar;
                ContentLength = PureSouceChar.size();
            }
            else
            {
                auto bodyPos = Source.find("\r\n\r\n");
                if (bodyPos == string::npos || bodyPos == Source.length() - 4)
                {
                    return;
                }
                BodyStr = Source.substr(bodyPos + 4);
                if (!IsChunked)
                {
                    return;
                }
                const auto& rb = BodyStr;
                MyString pureStr;
                // ��ȡ��һ��\r\n��λ��
                int NRpos = 0;
                auto GetNextNR = [&](int leap) {
                    NRpos = rb.find("\r\n", NRpos + leap);
                    return NRpos;
                };
                int Left = -2; // ����-2����Ϊ��һ��������400\r\n�����ģ�����������\r\n400\r\n������Ҫ�Ե�һ�ν��в���
                int Right = GetNextNR(0);
                while (Left != -1 && Right != -1)
                {
                    auto count = string(rb.begin() + 2 + Left, rb.begin() + Right); // ÿ���ֿ鿪ͷд������
                    if (count == "0")
                    {
                        break;
                    }                                           // ���һ�� 0\r\n\r\n���˳�
                    auto countNum = stoi(count, nullptr, 16);   // ��������16����
                    auto chunkedStart = rb.begin() + Right + 2; // ÿ���ֿ����ĵĿ�ʼλ��
                    pureStr.insert(pureStr.end(), chunkedStart, chunkedStart + countNum);
                    Left = GetNextNR(countNum); //  ����λ��
                    Right = GetNextNR(1);
                }
                BodyStr = pureStr;
                ContentLength = BodyStr.length();
            }
        }
    };

    class Request
    {
    public:
        int port = 80;
        MyString Source;
        MyString Method;
        MyString Path;
        MyString Protocol;
        MyString IP;
        MyString Url;
        MyString Host;
        MyString Cookie;
        MyString RequestBody;
        MyString ProtocolVersion = "HTTP/1.1";
        Header RequestHeader;
        Request() = default;
        ~Request() = default;

        Request& SetHttpMethod(sion::Method method)
        {
            std::lock_guard<std::mutex> lock(mu);
            switch (method)
            {
            case Method::Get:
                Method = "GET";
                break;
            case Method::Post:
                Method = "POST";
                break;
            case Method::Put:
                Method = "PUT";
                break;
            case Method::Delete:
                Method = "DELETE";
                break;
            }
            return *this;
        }

        Request& SetHttpMethod(MyString other)
        {
            std::lock_guard<std::mutex> lock(mu);
            Method = other;
            return *this;
        }

        Request& SetUrl(MyString url)
        {
            std::lock_guard<std::mutex> lock(mu);
            Url = url;
            return *this;
        }

        Request& SetCookie(MyString cookie)
        {
            std::lock_guard<std::mutex> lock(mu);
            Cookie = cookie;
            return *this;
        }

        Request& SetBody(MyString body)
        {
            std::lock_guard<std::mutex> lock(mu);
            RequestBody = body;
            return *this;
        }

        Request& SetHeader(vector<pair<MyString, MyString>> header)
        {
            std::lock_guard<std::mutex> lock(mu);
            RequestHeader.data = header;
            return *this;
        }

        Request& SetHeader(MyString k, MyString v)
        {
            std::lock_guard<std::mutex> lock(mu);
            RequestHeader.Add(k, v);
            return *this;
        }

        Response Send(sion::Method method, MyString url)
        {
            std::lock_guard<std::mutex> lock(mu);
            SetHttpMethod(method);
            return Send(url);
        }

        Response Send() { return Send(Url); }
#ifndef SION_DISABLE_SSL
    private:
        std::mutex mu;
        Response SendBySSL(Socket socket)
        {
            SSL_library_init();
            auto method = TLSv1_1_client_method();
            auto context = SSL_CTX_new(method);
            auto ssl = SSL_new(context);
            SSL_set_fd(ssl, socket);
            SSL_connect(ssl);
            SSL_write(ssl, Source.c_str(), Source.length());
            auto resp = ReadResponse(socket, ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(context);
            return resp;
        }
#endif
    public:
        Response Send(MyString url)
        {
            std::lock_guard<std::mutex> lock(mu);
            check<std::invalid_argument>(Method.length(), "���󷽷�δ����");
            std::smatch m;
#ifndef SION_DISABLE_SSL
            std::regex urlParse(R"(^(http|https)://([\w.]*):?(\d*)(/?.*)$)");
            regex_match(url, m, urlParse);
            check<std::invalid_argument>(m.size() == 5, "url��ʽ���Ի��������˳�http,https���Э��");
            Protocol = m[1];
            port = m[3].length() == 0 ? (Protocol == "http" ? 80 : 443) : stoi(m[3]);
#else
            std::regex urlParse(R"(^(http)://([\w.]*):?(\d*)(/?.*)$)");
            regex_match(url, m, urlParse);
            check<std::invalid_argument>(m.size() == 5, "url��ʽ���Ի��������˳�http���Э��");
            Protocol = m[1];
            port = m[3].length() == 0 ? 80 : stoi(m[3]);
#endif
            Host = m[2];
            Path = m[4].length() == 0 ? "/" : m[4].str();
            Socket socket = GetSocket();
            try
            {
                Connection(socket, Host);
                BuildRequestString();
                if (Protocol == "http")
                {
                    send(socket, Source.c_str(), int(Source.length()), 0);
                    return ReadResponse(socket);
                }
#ifndef SION_DISABLE_SSL
                else // if (Protocol == "https")
                {
                    return SendBySSL(socket);
                }
#endif
            }
            catch (const std::exception & e)
            {
                WSACleanup();
                throw e;
            }
        }
        auto SendAsync()
        {
            //std::lock_guard<std::mutex> lock(mu);
            return create_task([=] {
                std::lock_guard<std::mutex> lock(mu);
                return Send(Url); 
                });
        }

    private:
        void BuildRequestString()
        {
            RequestHeader.Add("Host", Host);
            RequestHeader.Add("Content-Length", std::to_string(RequestBody.length()));
            if (Cookie.length())
            {
                RequestHeader.Add("Cookie", Cookie);
            }
            Source = Method + " " + Path + " " + ProtocolVersion + "\r\n";
            for (auto& x : RequestHeader.data)
            {
                Source += x.first + ": " + x.second + "\r\n";
            }
            Source += "\r\n";
            Source += RequestBody;
        }

        void Connection(Socket socket, MyString host)
        {
            in_addr sa;
            IP = host.HasLetter() ? GetIpByHost(host) : host;
#ifdef UNICODE
            WCHAR wcIP[256];
            memset(wcIP, 0, sizeof(wcIP));
            MultiByteToWideChar(CP_ACP, 0, IP.c_str(), IP.length() + 1, wcIP, sizeof(wcIP) / sizeof(wcIP[0]));
            check<std::invalid_argument>(InetPton(AF_INET, wcIP, &sa) != -1, "��ַת������");
#else
            check<std::invalid_argument>((InetPton(AF_INET, IP.c_str(), &sa) != -1), "��ַת������");
#endif
            sockaddr_in saddr;
            saddr.sin_family = AF_INET;
            saddr.sin_port = htons(port);
            saddr.sin_addr = sa;
            if (::connect(socket, (sockaddr*)&saddr, sizeof(saddr)) != 0)
            {
                Throw<std::runtime_error>("����ʧ�ܴ����룺" + std::to_string(WSAGetLastError()));
            }
        }
#ifndef SION_DISABLE_SSL
        Response ReadResponse(Socket socket, SSL* ssl = nullptr)
#else
        Response ReadResponse(Socket socket)
#endif
        {
            const int bufSize = 2048;
            array<char, bufSize> buf{ 0 };
            auto Read = [&]() {
                buf.fill(0);
                int status = 0;
                if (Protocol == "http")
                {
                    status = recv(socket, buf.data(), bufSize - 1, 0);
                }
#ifndef SION_DISABLE_SSL
                else if (Protocol == "https")
                {
                    status = SSL_read(ssl, buf.data(), bufSize - 1);
                }
#endif
                check<std::runtime_error>(status >= 0, "�����쳣,Socket�����룺" + std::to_string(status));
                return status;
            };
            Response resp;
            // ��ȡ����ͷ����Ϣ
            auto ReadCount = Read();
            resp.Source += buf.data();
            resp.ParseFromSource(true);
            if (resp.SaveByCharVec)
            { // �ѳ�ͷ��������Ӧ�岿���ƹ�ȥ
                auto bodyPos = resp.Source.find("\r\n\r\n");
                auto startBody = buf.begin() + 4 + bodyPos;
                resp.BodyCharVec.insert(resp.BodyCharVec.end(), startBody, buf.begin() + ReadCount);
            }
            auto lenHeader = resp.Source.length() - resp.BodyStr.length(); // ��Ӧͷ����
            // ����Ƿ������
            auto CheckEnd = [&] {
                if (resp.SaveByCharVec)
                {
                    if (resp.IsChunked)
                    {
                        auto start = resp.BodyCharVec.begin() + resp.BodyCharVec.size() - 7;
                        return string(start, start + 7) == "\r\n0\r\n\r\n";
                    }
                    else
                    {
                        return resp.BodyCharVec.size() == resp.ContentLength;
                    }
                }
                else
                {
                    if (resp.IsChunked)
                    {
                        return resp.Source.substr(resp.Source.length() - 7) == "\r\n0\r\n\r\n";
                    }
                    else
                    {
                        return resp.Source.length() - lenHeader == resp.ContentLength;
                    }
                }
            };
            // ѭ����ȡ����
            while (!CheckEnd())
            {
                ReadCount = Read();
                if (resp.SaveByCharVec)
                {
                    resp.BodyCharVec.insert(resp.BodyCharVec.end(), buf.begin(), buf.begin() + ReadCount);
                }
                else
                {
                    resp.Source += buf.data();
                }
            }
            closesocket(socket);
            WSACleanup();
            resp.ParseFromSource();
            return resp;
        }
    };

    Response Fetch(MyString url, Method method = Method::Get, vector<pair<MyString, MyString>> header = {}, MyString body = "")
    {
        return Request().SetUrl(url).SetHttpMethod(method).SetHeader(header).SetBody(body).Send();
    }
    auto FetchAsync(MyString url, Method method = Method::Get, vector<pair<MyString, MyString>> header = {}, MyString body = "")
    {
        return Request().SetUrl(url).SetHttpMethod(method).SetHeader(header).SetBody(body).Send();
    }
} // namespace sion