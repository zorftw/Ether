#pragma once

#include <cstdint>

#include <jvmti.h>
#include <jni.h>
#include <jni_md.h>

#include <assert.h>
#include <string>

// Created with ReClass.NET 1.2 by KN4CK3R

namespace sdk {

	inline std::uintptr_t jvm_dll;
	bool init();


	struct c_vec3d {
		double x, y, z;
	};

	inline static double distance(double x, double y) {
		return sqrt(pow(x, 2) + pow(y, 2));
	}

	inline static double distance(double x1, double y1, double z1, double x2, double y2, double z2) {
		return distance(y1 - y2, distance(x1 - x2, z1 - z2));
	}

	class c_nativemethod
	{
	public:
		char pad_0000[8]; //0x0000
		class c_constantpool* constant_pool; //0x0008
		char pad_0010[176]; //0x0010
	}; //Size: 0x00C0

	template <typename T>
	class c_array
	{
	public:
		int32_t length; //0x0000
		T data[1]; //0x0004

		auto at(int i) const -> T {
			if (i >= 0 && i < length)
				return data[i];
			return (T)0;
		}

		auto is_empty() const -> bool {
			return length == 0;
		}

		T* adr_at(const int i) {
			if (i >= 0 && i < length)
				return &data[i];
			return nullptr;
		}
	}; //Size: 0x01F8

	inline int extract_low_short_from_int(jint x) {
		return x & 0xffff;
	}

	inline int extract_high_short_from_int(jint x) {
		return (x >> 16) & 0xffff;
	}

	class c_symbol
	{
	public:
		int16_t length; //0x0000
		int16_t identity; //0x0002
		char pad_0004[4]; //0x0004
		char text[1]; //0x0008
	}; //Size: 0x0018


	class c_constantpool
	{
	public:
		char pad_0000[8]; //0x0000
		c_array<uint8_t>* tags; //0x0008
		class c_constantpool_cache* constantpool_cache; //0x0010
		class c_instanceklass* instance_klass; //0x0018
		c_array<uint16_t>* operands; //0x0020
		c_array<c_instanceklass>* resolved; //0x0028
		uint16_t major; //0x0030
		uint16_t minor; //0x0031
		uint16_t generic_signature_index; //0x0032
		uint16_t source_file_name_index; //0x0033
		uint16_t flags; //0x0034
		int32_t length; //0x0035

		union {
			// set for CDS to restore resolved references
			int                _resolved_reference_length;
			// keeps version number for redefined classes (used in backtrace)
			int                _version;
		} _saved;
		class c_monitor* lock;

		intptr_t* base() const { return (intptr_t*)(((char*)this) + sizeof(c_constantpool)); }

		jlong long_at(int which)
		{
			uint32_t tmp = *(uint32_t*)(&base()[which]);
			return *((jlong*)&tmp);
		}

		jfloat* float_at_addr(int which)
		{
			return (jfloat*)&base()[which];
		}

		jdouble* double_at_addr(int which)
		{
			return (jdouble*)&base()[which];
		}

		intptr_t* obj_at_addr(int which)
		{
			return (intptr_t*)&base()[which];
		}

		jint* int_at_addr(int which)
		{
			return (jint*)&base()[which];
		}

		c_symbol* symbol_at(int which)
		{
			return *(c_symbol**)&base()[which];
		}

		jint name_and_type_at(int which) {
			return *int_at_addr(which);
		}

		int signature_ref_index_at(int which_nt) {
			jint ref_index = name_and_type_at(which_nt);
			return extract_high_short_from_int(ref_index);
		}

		int name_ref_index_at(int which_nt) {
			jint ref_index = name_and_type_at(which_nt);
			return extract_low_short_from_int(ref_index);
		}

		int impl_klass_ref_index_at(int which) 
		{
			jint ref_idex = *int_at_addr(which);
			return extract_low_short_from_int(ref_idex);
		}

		int impl_name_and_type_ref_index_at(int which)
		{
			jint ref_index = *int_at_addr(which);
			return extract_high_short_from_int(ref_index);
		}

		int uncached_klass_ref_index_at(int which) { return impl_klass_ref_index_at(which); }
		int uncached_name_and_type_ref_index_at(int which) { return impl_name_and_type_ref_index_at(which); }

	}; //Size: 0x0065

	// do something with this
	class c_name_and_type {
	private:
		int name_idx, sig_idx;
	public:
		c_name_and_type(int name, int sig) : name_idx(name), sig_idx(sig) {}

		c_symbol* get_name(c_constantpool* pool)
		{
			return pool->symbol_at(name_idx);
		}

		c_symbol* get_sig(c_constantpool* pool)
		{
			return pool->symbol_at(sig_idx);
		}
	};

#define FIELDINFO_TAG_SIZE             2
#define FIELDINFO_TAG_OFFSET           1 << 0
#define FIELDINFO_TAG_CONTENDED        1 << 1

	inline int build_int_from_shorts(std::uint16_t low, std::uint16_t high) {
		return ((int)((unsigned int)high << 16) | (unsigned int)low);
	}

	class c_fieldinfo {
	private:
		enum FieldOffset {
			access_flags_offset = 0,
			name_index_offset = 1,
			signature_index_offset = 2,
			initval_index_offset = 3,
			low_packed_offset = 4,
			high_packed_offset = 5,
			field_slots = 6
		};

	public:
		std::uint16_t _shorts[6];

		static c_fieldinfo* from_field_array(c_array<std::uint16_t>* fields, int index) {
			return ((c_fieldinfo*)fields->adr_at(index * field_slots));
		}

		void set_name_index(std::uint16_t val) { _shorts[name_index_offset] = val; }
		void set_signature_index(std::uint16_t val) { _shorts[signature_index_offset] = val; }
		void set_initval_index(std::uint16_t val) { _shorts[initval_index_offset] = val; }

		std::uint16_t name_index() const { return _shorts[name_index_offset]; }
		std::uint16_t signature_index() const { return _shorts[signature_index_offset]; }
		std::uint16_t initval_index() const { return _shorts[initval_index_offset]; }

		std::uint16_t access_flags() const { return _shorts[access_flags_offset]; }
		std::uint64_t offset() const {
			return build_int_from_shorts(_shorts[low_packed_offset], _shorts[high_packed_offset]) >> FIELDINFO_TAG_SIZE;
		}

		bool is_contended() const {
			return (_shorts[low_packed_offset] & FIELDINFO_TAG_CONTENDED) != 0;
		}

		std::uint16_t contended_group() const {
			assert((_shorts[low_packed_offset] & FIELDINFO_TAG_OFFSET) == 0, "Offset must not have been set");
			assert((_shorts[low_packed_offset] & FIELDINFO_TAG_CONTENDED) != 0, "Field must be contended");
			return _shorts[high_packed_offset];
		}

		bool is_offset_set() const {
			return (_shorts[low_packed_offset] & FIELDINFO_TAG_OFFSET) != 0;
		}

		c_symbol* name(c_constantpool* cp) const {
			int idx = name_index();

			return cp->symbol_at(idx);
		}

		c_symbol* sig(c_constantpool* cp) const {
			int idx = signature_index();

			return cp->symbol_at(idx);
		}

	};

	class c_instanceklass
	{
	public:
		char pad_0000[8]; //0x0000
		int32_t layout_helper; //0x0008
		int32_t super_check_offset; //0x000C
		class c_symbol* symbol; //0x0010
		class c_instanceklass* secondary_super_cache; //0x0018
		class c_array<c_instanceklass*>* secondary_super_array; //0x0020
		char pad_0028[64]; //0x0028
		class c_minecraft_fieldholder* static_fields; //0x0068
		class c_instanceklass* super; //0x0070
		class c_instanceklass* subklass; //0x0070
		class c_instanceklass* next_sibling; //0x0070
		class c_instanceklass* next_link; //0x0088
		class c_classloaderdata* classloader_data; //0x0090
		int32_t modifier_flags; //0x0098
		int32_t access_flags; //0x009C
		char pad_00A0[56]; //0x00A0
		class c_constantpool* constant_pool; //0x00D8
		char pad_00E0[143]; //0x00E0
		int8_t N0000081F; //0x016F
		char pad_0170[16]; //0x0170
		c_array<class c_method*>* methods; //0x0180
		c_array<class c_method*>* _default_methods; //0x0188
		void* _local_interfaces; //0x0190
		void* _transitive_interfaces; //0x0198
		void* _method_ordering; //0x01A0
		void* _default_vtable_indices; //0x01A8
		c_array<uint16_t>* fields; //0x01B0

	}; //Size: 0x01B0

	class c_weakhandle {};
	class c_oophandle {};

	class c_classloaderdata {
		static c_classloaderdata* null;
		class c_weakhandle holder;
		class c_oophandle class_loader;
		class c_classloadermetaspace* metaspace;
		class c_mutex* mutex;
		bool unloading;
		bool has_mirror;
		bool modified_oop;
		int keep_alive;
		volatile int claim;
	};

	

	std::string to_str(c_symbol* symbol);
	std::uint64_t find_offset(c_instanceklass* klass, std::string name, std::string sig);
	std::uint64_t find_offset(c_instanceklass* klass, c_symbol* name, c_symbol* sig);

	class c_minecraft_fieldholder
	{
	public:
		char pad_0000[96]; //0x0000
		int32_t num_statistics; //0x0060
		char pad_0064[20]; //0x0064
		int32_t instance; //0x0078
		char pad_007C[4]; //0x007C
	}; //Size: 0x0080

	class N000004E1
	{
	public:
		char pad_0000[8]; //0x0000
	}; //Size: 0x0008

	class c_method
	{
	public:
		char pad_0000[8]; //0x0000
		class c_constmethod* const_method; //0x0008
		void* N000004BF; //0x0010
		void* N000004C0; //0x0018
		class N000004E1 N000004C1; //0x0020
		int32_t vtable_index; //0x0028
		int16_t intrinsic_id; //0x002C
		int16_t flags; //0x002E
		char pad_0030[209]; //0x0030
	}; //Size: 0x0101

	class c_constantpool_cache
	{
	public:
		char pad_0000[4]; //0x0000
		int32_t length; //0x0004
		class c_constantpool* constant_pool; //0x0008
	}; //Size: 0x0010

	class c_constmethod
	{
	public:
		enum {
			_has_linenumber_table = 0x0001,
			_has_checked_exceptions = 0x0002,
			_has_localvariable_table = 0x0004,
			_has_exception_table = 0x0008,
			_has_generic_signature = 0x0010,
			_has_method_parameters = 0x0020,
			_is_overpass = 0x0040,
			_has_method_annotations = 0x0080,
			_has_parameter_annotations = 0x0100,
			_has_type_annotations = 0x0200,
			_has_default_annotations = 0x0400
		};
		int64_t fingerprint; //0x0000
		class c_constantpool* constant_pool; //0x0008
		c_array<uint8_t>* stackmap_data; //0x0010

		 // Adapter blob (i2c/c2i) for this Method*. Set once when method is linked.
		union {
			class c_adapterhandlerentry* _adapter;
			class c_adapterhandlerentry** _adapter_trampoline; // see comments around Method::link_method()
		};

		int32_t constmethod_size; //0x0018
		int16_t flags; //0x001C
		int8_t result_type; //0x001E
		int16_t code_size; //0x001F
		int16_t name_index; //0x0021
		int16_t sig_index; //0x0023
		int16_t method_id_num; //0x0025
		int16_t max_stack; //0x0027
		int16_t max_locals; //0x0029
		int16_t num_params; //0x002B
		int16_t orig_method_idnum; //0x002D
	};

	class c_jvm_array
	{
	private:
		char pad_0000[16]; //0x0000
	public:
		int32_t length; //0x0010
		int32_t ptr_to_array; //0x0014

		std::uint32_t get_array_offset() const {
			return 0x10;
		}

		std::uint32_t get_at(int which) const {
			if (which > length)
				return 0;

			auto addr_to_read = ptr_to_array + get_array_offset() + (which * sizeof(std::uint32_t));

			return *reinterpret_cast<std::uint32_t*>(addr_to_read);
		}

		template <typename T>
		T at(int which) const {
			if (which > length)
				return nullptr;

			return reinterpret_cast<T>(get_at(which));
		}

	}; //Size: 0x0014


	class N00000853
	{
	public:
		void* N00000854; //0x0000
		char pad_0008[32]; //0x0008
		int32_t N00000859; //0x0028
		char pad_002C[4]; //0x002C
		int8_t bytecode[1]; //0x0030
		char pad_0031[264]; //0x0031
	}; //Size: 0x0139

	enum java_thread_state {
		_thread_uninitialized = 0, // should never happen (missing initialization)
		_thread_new = 2, // just starting up, i.e., in process of being initialized
		_thread_new_trans = 3, // corresponding transition state (not used, included for completness)
		_thread_in_native = 4, // running in native code
		_thread_in_native_trans = 5, // corresponding transition state
		_thread_in_vm = 6, // running in VM
		_thread_in_vm_trans = 7, // corresponding transition state
		_thread_in_Java = 8, // running in Java or in stub code
		_thread_in_Java_trans = 9, // corresponding transition state (not used, included for completness)
		_thread_blocked = 10, // blocked in vm
		_thread_blocked_trans = 11, // corresponding transition state
		_thread_max_state = 12  // maximum thread state+1 - used for statistics allocation
	};

	c_instanceklass* find_klass(const char* name);

	class c_jvmthread {
	public:

		// iteration
		static c_jvmthread* first();
		c_jvmthread* next();

		// environment
		JNIEnv* get_environment();

		// utilities
		bool is_compiler_thread();
		bool is_java_thread();

		static c_jvmthread* get_thread();
		static void set_thread(c_jvmthread* thread);

		//java_thread_state get_thread_state();
	};

}