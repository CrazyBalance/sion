#pragma once
#pragma warning(disable:4267)
#pragma warning(disable:4018)
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <vector>
#include<WS2tcpip.h>
#include<map>
#pragma comment(lib,"ws2_32.lib")  //2


class MyString :public std::string
{
public:
	MyString() = default;
	~MyString() = default;
	template<class T>
	MyString(T&& arg) :std::string(std::forward<T>(arg))
	{

	}

	//ʹ���ַ����ָ�
	//flag �ָ��־,���ص��ַ����������޳�,flag��Ҫ��char�������ز���ȷ
	//num �ָ������Ĭ��0���ָ��������num=1,���ؿ�ͷ��flag,flag������size=2���ַ�������
	//skipEmpty �������ַ���������ѹ��length==0���ַ���
	std::vector<MyString> Split(MyString flag, int num = 0, bool skipEmpty = true)
	{
		std::vector<MyString> dataSet;
		auto PushData = [&dataSet, skipEmpty](MyString line)
		{
			if (line.length() != 0 || !skipEmpty)
			{
				dataSet.push_back(line);
			}
		};
		auto Pos = FindAll(flag, num != 0 ? num : -1);
		if (!Pos.size())
		{
			return dataSet;
		}
		for (int i = 0; i < Pos.size(); i++)
		{
			if (dataSet.size() == num && num != 0)
			{	//��������ֱ�ӽص�����
				PushData(substr(Pos[dataSet.size()] + flag.size()));
				break;
			}
			if (i == 0 && Pos[0] != 0)
			{	//��һ������λ�ò���0�Ļ�����
				PushData(substr(0, Pos[0]));
			}
			if (i != Pos.size() - 1)
			{
				int Left = Pos[i] + flag.length();
				int Right = Pos[i + 1] - Left;
				PushData(substr(Left, Right));
			}
			else
			{	//���һ����־������
				PushData(substr(*(--Pos.end()) + flag.size()));
			}
		}
		return dataSet;
	}

	//���ǰ����ַ�
	//target ��Ҫ������ַ�Ĭ�Ͽո�
	MyString Trim(char target = ' ')
	{
		auto left = find_first_not_of(target);
		auto right = find_last_not_of(target);
		return substr(left, right - left + 1);
	}

	//������ĸ
	bool HasLetter()
	{
		for (auto x : *this)
		{
			if ((x >= 'a'&&x <= 'z') ||
				(x >= 'A'&&x <= 'Z'))
			{
				return true;
			}
		}
		return false;
	}

	//ת����gbk ������ʾ����������
	MyString ToGbk()
	{
		//��blog.csdn.net/u012234115/article/details/83186386 �Ĺ���
		auto src_str = c_str();
		int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
		wchar_t* wszGBK = new wchar_t[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
		len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
		char* szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
		MyString result(szGBK);
		if (wszGBK) delete[] wszGBK;
		if (szGBK) delete[] szGBK;
		return result;
	}



	//����������������λ��
	//flag ��λ��־
	//num ����������Ĭ��ֱ������
	std::vector<int> FindAll(MyString flag, int num = -1)
	{
		std::vector<int> Result;
		auto Pos = find(flag);
		while (Pos != -1 && Result.size() != num)
		{
			Result.push_back(Pos);
			Pos = find(flag, *(--Result.end()) + 1);
		}
		return Result;
	}


	//�ַ����滻
	MyString& Replace(MyString oldStr, MyString newStr)
	{
		int pos = find(oldStr);
		if (pos != -1)
		{
			replace(pos, oldStr.length(), newStr);
		}
		return *this;
	}

};

namespace Sion
{
	using Socket = SOCKET;
	MyString GetIpByHost(MyString hostname)
	{
		addrinfo hints, *res;
		in_addr addr;
		int err;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_family = AF_INET;//ipv4
		if ((err = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0) {
			MyString Msg = "����" + err + MyString(gai_strerror(err));
			throw std::exception(Msg.c_str());
		}
		addr.s_addr = ((sockaddr_in*)(res->ai_addr))->sin_addr.s_addr;
		char str[INET_ADDRSTRLEN];
		auto ptr = inet_ntop(AF_INET, &addr, str, sizeof(str));
		freeaddrinfo(res);
		return str;
	}

	Socket GetSocket()//http������Ҫ��˿�
	{
		//��ʼ����,WSA windows�첽�׽���
		WSADATA inet_WsaData;//
		WSAStartup(MAKEWORD(2, 0), &inet_WsaData);//socket2.0�汾
		if (LOBYTE(inet_WsaData.wVersion) != 2 || HIBYTE(inet_WsaData.wVersion) != 0)
		{//��λ�ֽ�ָ�����汾����λ�ֽ�ָ�����汾
			WSACleanup();
			return -1;
		}
		auto tcp_socket = socket(AF_INET, SOCK_STREAM, 0);//ipv4,tcp,tcp��udp�ò�����Ϊ0
		return tcp_socket;
	}

	enum MethodEnum
	{
		Get,
		Post,
		Put,
		Delete
	};

	class Response
	{
	public:
		MyString Source;
		MyString Cookie;
		MyString ProtocolVersion = "HTTP/1.1";
		MyString Code = "200";
		MyString Status = "ok";
		MyString Server = "CppTinyServer";
		MyString TransferEncoding;
		int ContentLength = 0;
		bool IsChunked;
		std::map<MyString, MyString> Header;
		MyString ResponseBody;
		~Response() = default;
		Response() = default;

		template<class T>
		Response(T&& arg)
		{
			Source = MyString(std::forward<T>(arg));
		}


		//�������������͹�������Ӧ
		MyString ParseFromSource(bool ConverToGbk = false)
		{
			if (ConverToGbk)
			{
				Source = Source.ToGbk();
			}
			auto ThrowError = [this]
			{
				MyString Msg = "��������" + Source;
				throw std::exception(Msg.c_str());
			};
			auto HeaderStr = Source.substr(0, Source.find("\r\n\r\n"));
			auto data = MyString(std::move(HeaderStr)).Split("\r\n");
			if (data.size() == 0)
			{
				return ResponseBody;
			}
			auto FirstLine = data[0].Split(" ");
			if (FirstLine.size() != 3)
			{
				ThrowError();
			}
			ProtocolVersion = FirstLine[0].Trim();
			Code = FirstLine[1].Trim();
			Status = FirstLine[2].Trim();
			data.erase(data.begin());
			for (auto x : data)
			{
				auto pair = x.Split(":", 1);
				if (pair.size() == 2)
				{
					Header[pair[0].Trim()] = pair[1].Trim();
				}
			}
			MyString contentLen = Header["Content-Length"];
			ContentLength = contentLen != "" ? stoi(contentLen) : ContentLength;
			Cookie = Header["Cookie"];
			Server = Header["Server"];
			TransferEncoding = Header["Transfer-Encoding"];
			IsChunked = ContentLength == 0;
			auto bodyPos = Source.find("\r\n\r\n");
			if (bodyPos != -1 && bodyPos != Source.length() - 4)
			{
				ResponseBody = Source.substr(bodyPos + 4);
				if (IsChunked)
				{
					auto Pos = ResponseBody.FindAll("\r\n");
					MyString ResponseBodyClear;
					for (int i = 0; i < (Pos.size() - 1) / 2; i++)
					{
						int Left = Pos[i * 2] + 2;
						int Right = Pos[i * 2 + 1] - Left;
						ResponseBodyClear += ResponseBody.substr(Left, Right);
					};
					ResponseBody = ResponseBodyClear;
				}
			}
			return ResponseBody;
		}

		MyString& BuildResponseString()
		{
			Header["Cookie"] = Cookie;
			Header["Server"] = Server;
			Header["Content-Length"] = std::to_string(ResponseBody.length());
			Source = ProtocolVersion + " " + Code + " " + Status + "\r\n";
			for (auto x : Header)
			{
				Source += x.first + ":" + x.second + "\r\n";
			}
			Source += "\r\n";
			Source += ResponseBody;
			return Source;
		}

	};

	class Request
	{
	public:
		MyString Source;
		MyString Method;
		MyString Path;
		MyString ProtocolVersion = "HTTP/1.1";
		MyString IP;
		MyString Host;
		MyString Cookie;
		MyString RequestBody;
		std::map<MyString, MyString> Header;
		Request() = default;
		~Request() = default;

		template<class T>
		Request(T&& arg)
		{
			Source = MyString(std::forward<T>(arg));
		}

		void SetHttpMethod(MethodEnum method, MyString other = "")
		{
			if (other != "")
			{
				Method = other;
				return;
			}
			switch (method)
			{
			case MethodEnum::Get:
				Method = "GET";
				break;
			case MethodEnum::Post:
				Method = "POST";
				break;
			case MethodEnum::Put:
				Method = "PUT";
				break;
			case MethodEnum::Delete:
				Method = "DELETE";
				break;
			default:
				break;
			}
		}

		void Connection(Socket socket, MyString host, bool IsSSL = false)
		{
			int port = IsSSL ? 433 : 80;//û��:8080��ֱ��80����
			auto indexPort = host.find(":");
			if (indexPort != -1)
			{//127.0.0.1��5000���Ķ˿�Ϊ5000
				port = stoi(host.substr(indexPort + 1));
				host = host.substr(0, indexPort);//ȥ���˿�
			}
			in_addr sa;
			IP = host.HasLetter() ? GetIpByHost(host) : host;
			if (InetPton(AF_INET, IP.c_str(), &sa) == -1)
			{
				throw new std::exception("��ַת������");
			}
			sockaddr_in saddr;
			saddr.sin_family = AF_INET;
			saddr.sin_port = htons(port);
			saddr.sin_addr = sa;
			if (::connect(socket, (sockaddr*)&saddr, sizeof(saddr)) != 0)
			{
				MyString Msg = "����ʧ�ܴ����룺" +std::to_string(WSAGetLastError());
				throw std::exception(Msg.c_str());
			}
		}

		//�����ͻ��˷��͹���������
		void ParseFromSource()
		{
			auto ThrowError = [this]
			{
				MyString Msg = "��������" + Source;
				throw std::exception(Msg.c_str());
			};
			if (Source.length() < 10)
			{
				ThrowError();
			}
			auto HeaderStr = Source.substr(0, Source.find("\r\n\r\n"));
			auto data = MyString(std::move(HeaderStr)).Split("\r\n");
			if (data.size() == 0)
			{
				ThrowError();
			}
			//������
			auto FirstLine = data[0].Split(" ");
			if (FirstLine.size() != 3)
			{
				ThrowError();
			}
			Method = FirstLine[0].Trim();
			Path = FirstLine[1].Trim();
			ProtocolVersion = FirstLine[2].Trim();
			data.erase(data.begin());
			//����ͷ
			Header.clear();
			for (auto x : data)
			{
				auto pair = x.Split(":", 1);
				if (pair.size() == 2)
				{
					Header[pair[0].Trim()] = pair[1].Trim();
				}
			}
			//������
			auto bodyPos = Source.find("\r\n\r\n");
			if (bodyPos != -1 && bodyPos != Source.length() - 4)
			{
				RequestBody = Source.substr(bodyPos + 4);
			}
			Cookie = Header["Cookie"];
			Host = Header["Host"];
		}

		void BuildRequestString()
		{
			Header["Cookie"] = Cookie;
			Header["Host"] = Host;
			if (RequestBody != "")
			{
				Header["Content-Length"] = std::to_string(RequestBody.length());
			}
			Source = Method + " " + Path + " " + ProtocolVersion + "\r\n";
			for (auto x : Header)
			{
				Source += x.first + ":" + x.second + "\r\n";
			}
			Source += "\r\n";
			Source += RequestBody;
		}

		MyString SendRequest(MyString url)
		{
			if (Method.length() == 0)
			{
				throw std::exception("δ�������󷽷�");
			}
			//����url����
			MyString protocolStr = "http://";
			if (url.find(protocolStr) != 0)
			{
				MyString Msg = "����Э�飡������ʹ��" + protocolStr;
				throw std::exception(Msg.c_str());
			}
			url.erase(0, protocolStr.length());
			Host = url.substr(0, url.find("/"));
			url.erase(0, Host.length());
			Path = url.length() != 0 ? url : "/";//��ǰ���Ѿ���httpЭ��ͷ��������ip��ȥ
			//
			Socket socket = GetSocket();
			Connection(socket, Host);
			BuildRequestString();
			send(socket, Source.c_str(), int(Source.length()), 0);
			return ReadResponse(socket);
		}


		MyString SendRequest(MethodEnum method, MyString url)
		{
			SetHttpMethod(method);
			return SendRequest(url);
		}

		MyString static StaticRequest(MethodEnum method, MyString url)
		{
			Request request;
			request.SetHttpMethod(method);
			return request.SendRequest(url);
		}

	private:

		MyString ReadResponse(Socket socket)
		{
			char buf[1024] = { 0 };
			Response resp;
			recv(socket, buf, sizeof(buf) - 1, 0);
			resp.Source += buf;
			resp.ParseFromSource();
			auto lenHeader = resp.Source.length() - resp.ResponseBody.length();//��Ӧͷ����
			while (true)
			{	//���յ����ַ����ڻ��������ȣ����ս���,Ҳ�п����Ǵ���������ӹر�
				if (resp.IsChunked)
				{
					if (resp.Source.substr(resp.Source.length() - 7) == "\r\n0\r\n\r\n")
					{
						break;
					}
				}
				else
				{
					if (resp.Source.length() - lenHeader == resp.ContentLength)
					{
						break;
					}
				}
				memset(buf, 0, sizeof(buf));
				if (int num = recv(socket, buf, sizeof(buf) - 1, 0) < 0)
				{
					MyString Msg = "�����쳣,Socket�����룺" +std::to_string(num);
					throw std::exception(Msg.c_str());
				}
				resp.Source += buf;
			}
			closesocket(socket);
			return resp.Source;
		}
	};

}