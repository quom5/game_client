#ifndef __GAMECLIENT_RPC
#define __GAMECLIENT_RPC 1

#include <tuple>
#include <iostream> 
#include <istream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <math.h>

// --- process single datatype ---
#define RPC_NUM_TYPES 18
const int AnySize[RPC_NUM_TYPES] = {
	1, 1,
	sizeof(char), sizeof(unsigned char), sizeof(short),
	sizeof(ushort), sizeof(int), sizeof(uint),
	sizeof(double), sizeof(float),
//	sizeof(vec2), sizeof(vec3), sizeof(vec4), sizeof(quat),
//	sizeof(mat2), sizeof(mat3), sizeof(mat4),
	1
};
const std::string AnyStr[RPC_NUM_TYPES] = {
	"String", "Vector",
	"Char", "UChar", "Short",
	"UShort", "Int", "UInt",
	"Double", "Float",
	"Vec2", "Vec3", "Vec4", "Quat",
	"Mat2", "Mat3", "Mat4", "Map"
};

class Rpc
{
public:
	Rpc(){ _recv_data.clear(); _send_data.clear(); }
	// ------------- UTILITY---------------
	template<int...> struct index_tuple{};

	template<int I, typename IndexTuple, typename... Types>
	struct make_indexes_impl;

	template<int I, int... Indexes, typename T, typename ... Types>
	struct make_indexes_impl < I, index_tuple<Indexes...>, T, Types... >
	{
		typedef typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>, Types...>::type type;
	};

	template<int I, int... Indexes>
	struct make_indexes_impl < I, index_tuple<Indexes...> >
	{
		typedef index_tuple<Indexes...> type;
	};

	template<typename ... Types>
	struct make_indexes : make_indexes_impl < 0, index_tuple<>, Types... > {};




	struct Any {
		enum Type {
			String = 0, Vector = 1,
			Char = 2, UChar = 3, Short = 4,
			UShort = 5, Int = 6, UInt = 7,
			Double = 8, Float = 9,
			Vec2 = 10, Vec3 = 11, Vec4 = 12, Quat = 13,
			Mat2 = 14, Mat3 = 15, Mat4 = 16,
		};
		union All
		{
			double d; float f; char c;
			short s; unsigned short us;
			int i; uint ui;
			unsigned char uc[16];
		};
		Any(void){ _type = 0; };
		Any(double e)
		{
			_num = e; _type = Double;
			if (e - double(char(e)) == 0) { _type = Char; return; }
			if (e - double(unsigned char(e)) == 0) { _type = UChar; return; }
			if (e - double(short(e)) == 0) { _type = Short; return; }
			if (e - double(ushort(e)) == 0) { _type = UShort; return; }
			if (e - double(int(e)) == 0) { _type = Int; return; }
			if (e - double(uint(e)) == 0) { _type = UInt; return; }
			if (e - double(float(e)) == 0) { _type = Float; return; }
		}
		Any(std::string e) { _str = e; _type = String; }
		//Any(vec2 v){ _f[0] = v.x; _f[1] = v.y;  _type = Vec2; };
		//Any(vec3 v){ _f[0] = v.x; _f[1] = v.y; _f[2] = v.z;  _type = Vec3; };
		//Any(vec4 v){ _f[0] = v.x; _f[1] = v.y; _f[2] = v.z; _f[3] = v.w;  _type = Vec4; };
		//Any(quat v){ _f[0] = v.x; _f[1] = v.y; _f[2] = v.z; _f[3] = v.w;  _type = Quat; };
		//Any(mat2 m){ _type = Mat2; loopi(0, 2)loopj(0, 2)_f[j * 2 + i] = m[j][i]; };
		//Any(mat3 m){ _type = Mat3; loopi(0, 3)loopj(0, 3)_f[j * 3 + i] = m[j][i]; };
		//Any(mat4 m){ _type = Mat4; loopi(0, 4)loopj(0, 4)_f[j * 4 + i] = m[j][i]; };

		template<class T>
		Any(std::vector<T> v)
		{
			_vec.clear();
			for (size_t i = 0; i < v.size(); i++) { _vec.push_back(v[i]); }
			_type = Vector;
		};

		int get_type() const { return _type; }
		std::string get_type_string(){ return AnyStr[_type]; }
		std::string get_data_as_string()
		{
			std::ostringstream s;
			if (_type == Char || _type == UChar
				|| _type == Short || _type == UShort
				|| _type == Int || _type == UInt)	s << (int)_num;
			if (_type == Float) s << (float)_num;
			if (_type == Double) s << (double)_num;
			//if (_type == Vec2) loopi(0, 2) { s << _f[i] << " "; };
			//if (_type == Vec3) loopi(0, 3) { s << _f[i] << " "; };
			//if (_type == Vec4) loopi(0, 4) { s << _f[i] << " "; };
			//if (_type == Quat) loopi(0, 4) { s << _f[i] << " "; };
			//if (_type == Mat2) loopj(0, 2) loopi(0, 2) { s << _f[j * 2 + i] << " "; };
			//if (_type == Mat3) loopj(0, 3) loopi(0, 3) { s << _f[j * 2 + i] << " "; };
			//if (_type == Mat4) loopj(0, 4) loopi(0, 4) { s << _f[j * 2 + i] << " "; };
			if (_type == String) s << _str;
			if (_type == Vector) { s << "[ "; for (size_t i = 0; i < _vec.size(); i++) { s << _vec[i].get_data_as_string() << " "; }; s << "]"; }
			return s.str();
		}
		void net_push(std::vector<unsigned char> &n)
		{
			//cout << get_type_string() << endl;
			All a; int size = 0;
			n.push_back((int)_type);
			if (_type == Char){ a.c = (char)_num; size = sizeof(char); }
			if (_type == UChar){ a.uc[0] = (unsigned char)_num; size = sizeof(unsigned char); }
			if (_type == Short) { a.s = (short)_num; size = sizeof(short); }
			if (_type == UShort){ a.us = (unsigned short)_num; size = sizeof(unsigned short); }
			if (_type == Int) { a.i = (int)_num; size = sizeof(int); }
			if (_type == UInt) { a.ui = (uint)_num; size = sizeof(uint); }
			if (_type == Float) { a.f = (float)_num; size = sizeof(float); }
			if (_type == Double) { a.d = (double)_num; size = sizeof(double); }
			if (size > 0) for(size_t i = 0; i < size; i++) { n.push_back(a.uc[i]); }
			if (_type >= Vec2) if (_type <= Mat4)
				for (size_t i = 0; i < AnySize[_type]; i++) { n.push_back(((unsigned char*)_f)[i]); }
			if (_type == String) { for (size_t i = 0; i < _str.size(); i++) { n.push_back(_str[i]); n.push_back(0); } }
			if (_type == Vector) { Any len(_vec.size()); len.net_push(n); for (size_t i = 0; i < _vec.size(); i++) { _vec[i].net_push(n); } }
		}
		int net_pop(std::vector<unsigned char> &n, int index = 0)
		{
			All a; int size = 0;
			_type = n[index]; index++;
			if (_type > RPC_NUM_TYPES || _type<0 ) error_stop("Any::Unknown Type [%d]", _type);
			if (_type == String)
			{
				_str = ((char*)&n[index]);
				return index + _str.size() + 1;
			}
			if (_type == Vector)
			{
				Any len; index = len.net_pop(n, index);
				_vec.resize(int(len._num));
				for (size_t i = 0; i < _vec.size(); i++) { index = _vec[i].net_pop(n, index); }
				return index;
			}
			if (_type >= Char)if (_type <= Float)
			{
				int size = AnySize[_type];
				for (size_t i = 0; i < size; i++) { a.uc[i] = n[index + i]; }
				if (_type == UChar) _num = a.uc[0];
				if (_type == Char) _num = a.c;
				if (_type == Short) _num = a.s;
				if (_type == UShort) _num = a.us;
				if (_type == Int) _num = a.i;
				if (_type == UInt) _num = a.ui;
				if (_type == Float) _num = a.f;
				if (_type == Double) _num = a.d;
				return index + size;
			}
			if (_type >= Vec2)if (_type <= Mat4)
			{
				for (size_t i = 0; i < AnySize[_type]; i++) { ((unsigned char*)_f)[i] = n[index + i]; }
				return index + AnySize[_type];
			}
		}
		//vector<Any> get_vector() const { return _vec; }

		void getT(unsigned char &a){ a = (unsigned char)_num; };
		void getT(char &a){ a = (char)_num; };
		void getT(short &a){ a = (short)_num; };
		void getT(ushort &a){ a = (ushort)_num; };
		void getT(float &a){ a = (float)_num; };
		void getT(double &a){ a = (double)_num; };
		void getT(int &a){ a = (int)_num; };
		void getT(uint &a){ a = (uint)_num; };
		//void getT(vec2 &a){ a = vec2(_f[0], _f[1]); };
		//void getT(vec3 &a){ a = vec3(_f[0], _f[1], _f[2]); };
		//void getT(vec4 &a){ a = vec4(_f[0], _f[1], _f[3], _f[4]); };
		//void getT(quat &a){ a = quat(_f[0], _f[1], _f[3], _f[4]); };
		//void getT(mat2 &m){ loopi(0, 2)loopj(0, 2) m[j][i] = _f[j * 2 + i]; };
		//void getT(mat3 &m){ loopi(0, 3)loopj(0, 3) m[j][i] = _f[j * 3 + i]; };
		//void getT(mat4 &m){ loopi(0, 4)loopj(0, 4) m[j][i] = _f[j * 4 + i]; };
		void getT(std::string &a){ a = _str; };
		template<class T>
		void getT(std::vector<T> &v)
		{
			v.clear();
			for(size_t i = 0; i < _vec.size(); i++)
			{
				T t;
				_vec[i].getT(t);
				v.push_back(t);
			}
		};
		int get_type(){ return _type; }
	private:
		int _type;
		std::string _str;
		double _num;
		float _f[16];
		std::vector<Any> _vec;
	};

	template <typename T> T static read_from(int index, std::vector<Any> &list)
	{
		T t; list[index].getT(t); return t;
	}

	// ---- RPC Data to Params ----


	std::map<std::string, std::function<void(std::vector<unsigned char>&, int&)> > functionarray;

	template<typename... Args, class F, int... Is>
	void RegisterRPC_index(std::string name, F f, index_tuple< Is... >)
	{
		const int n = sizeof...(Args);
		std::cout << "[" << n << " args]\n";

		functionarray[name] = [f](std::vector<unsigned char> &data, int &index)
		{
			const int n = sizeof...(Args);
			std::cout << n << " args\n";

			std::vector<Any> list;

			for (int i = 0; i < n; i++)
			{
				Any a;
				index = a.net_pop(data, index);
				list.push_back(a);
			}
			f(read_from<Args>(Is, list)...);
		};
	}

	template< typename... Args, class F, int... Is>
	void RegisterRPC_index_ret(std::string name, F f, index_tuple< Is... >)
	{
		const int n = sizeof...(Args);
		std::cout << "[" << n << " args]\n";

		functionarray[name] = [f](std::vector<unsigned char> &data, int &index)
		{
			const int n = sizeof...(Args);
			std::cout << n << " args\n";

			std::vector<Any> list;

			for (int i = 0; i < n; i++)
			{
				Any a;
				index = a.net_pop(data, index);
				list.push_back(a);
			}
			//vector<uchar> network_send;
			data.clear();
			Any any(f(read_from<Args>(Is, list)...));
			any.net_push(data);
		};
	}

	template<typename... Args, class F>
	void RegisterRPC_no_ret(std::string name, F f)
	{
		RegisterRPC_index<Args...>(name, f, typename make_indexes<Args...>::type());
	}

	template<typename... Args, class F>
	void RegisterRPC_ret(std::string name, F f)
	{
		RegisterRPC_index_ret<Args...>(name, f, typename make_indexes<Args...>::type());
	}

	// --- RPC Params to Data ---


	std::vector<unsigned char> _send_data, _recv_data;

	template <class ...Args>
	void CallRPC_imp(const Args&... args)
	{
		std::vector<Any> vec = { args... };

		for (size_t i = 0; i < vec.size(); i++)
		{
			vec[i].net_push(_send_data);
		};
	};

	template <class ...Args> void call_void(std::string name, Args... args)
	{
		const int n = sizeof...(Args);
		std::cout << "calling " << name << " " << n << " args" << std::endl;
		CallRPC_imp(Any(name), Any(args)...);
	};

	void process()
	{
		Any any; int index = 0; std::string s;

		while (index < _recv_data.size())
		{
			index = any.net_pop(_recv_data, index);
			any.getT(s);
			if (functionarray.find(s) != functionarray.end())
				functionarray[s](_recv_data, index);
			else
				error_stop("Server RPC function %s not found", s.c_str());
		}
		_recv_data.clear();
	}
	void process_ret()
	{
		Any any; int index = 0; std::string s;

		while (index < _recv_data.size())
		{
			index = any.net_pop(_recv_data, index);
			any.getT(s);
			if (functionarray.find(s) != functionarray.end())
				functionarray[s](_recv_data, index);
			else
				error_stop("Server RPC function %s not found", s.c_str());
		}
		_send_data = _recv_data;
		_recv_data.clear();
	}
	// traits helper templates
	template<typename T>
	struct function_traits;
	template<typename R, typename ...Args>
	
	struct function_traits < std::function<R(Args...)> >
	{
		static const size_t nargs = sizeof...(Args);
		typedef R result_type;
		template <size_t i>
		struct arg
		{
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
		};
		
		template <bool B> class Foo;
		
		template <> class Foo<true>
		{
		
		public:
			static void set_rpc(Rpc *rpc,std::string name, std::function<R(Args...)> f)
			{
				std::cout << "[non blocking/void] ";
				rpc->RegisterRPC_no_ret<Args...>(name, f);
			}
		};
		
		template <> class Foo<false>
		{
		public:
			static void set_rpc(Rpc *rpc, std::string name, std::function<R(Args...)> f)
			{
				std::cout << "[blocking/non void] ";
				rpc->RegisterRPC_ret<Args...>(name, f);
			}
		};
	};

};//

// --------------------- defines ------------------
#define rpc_register(grpc,a)\
{\
	typedef std::function<decltype(a)> fun;\
	typedef Rpc::function_traits<fun> fun2;\
	cout << "rpc_register " << #a << " ";\
	fun2::Foo<is_same<void,fun2::result_type>::value>::set_rpc(&grpc,#a, a);\
}

#endif
