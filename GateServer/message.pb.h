// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: message.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_message_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_message_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3013000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3013000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_message_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_message_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_message_2eproto;
namespace message {
class VerifyRequst;
class VerifyRequstDefaultTypeInternal;
extern VerifyRequstDefaultTypeInternal _VerifyRequst_default_instance_;
class VerifyResponse;
class VerifyResponseDefaultTypeInternal;
extern VerifyResponseDefaultTypeInternal _VerifyResponse_default_instance_;
}  // namespace message
PROTOBUF_NAMESPACE_OPEN
template<> ::message::VerifyRequst* Arena::CreateMaybeMessage<::message::VerifyRequst>(Arena*);
template<> ::message::VerifyResponse* Arena::CreateMaybeMessage<::message::VerifyResponse>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace message {

// ===================================================================

class VerifyRequst PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:message.VerifyRequst) */ {
 public:
  inline VerifyRequst() : VerifyRequst(nullptr) {}
  virtual ~VerifyRequst();

  VerifyRequst(const VerifyRequst& from);
  VerifyRequst(VerifyRequst&& from) noexcept
    : VerifyRequst() {
    *this = ::std::move(from);
  }

  inline VerifyRequst& operator=(const VerifyRequst& from) {
    CopyFrom(from);
    return *this;
  }
  inline VerifyRequst& operator=(VerifyRequst&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const VerifyRequst& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const VerifyRequst* internal_default_instance() {
    return reinterpret_cast<const VerifyRequst*>(
               &_VerifyRequst_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(VerifyRequst& a, VerifyRequst& b) {
    a.Swap(&b);
  }
  inline void Swap(VerifyRequst* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(VerifyRequst* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline VerifyRequst* New() const final {
    return CreateMaybeMessage<VerifyRequst>(nullptr);
  }

  VerifyRequst* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<VerifyRequst>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const VerifyRequst& from);
  void MergeFrom(const VerifyRequst& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(VerifyRequst* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "message.VerifyRequst";
  }
  protected:
  explicit VerifyRequst(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_message_2eproto);
    return ::descriptor_table_message_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kEmailFieldNumber = 1,
  };
  // string email = 1;
  void clear_email();
  const std::string& email() const;
  void set_email(const std::string& value);
  void set_email(std::string&& value);
  void set_email(const char* value);
  void set_email(const char* value, size_t size);
  std::string* mutable_email();
  std::string* release_email();
  void set_allocated_email(std::string* email);
  private:
  const std::string& _internal_email() const;
  void _internal_set_email(const std::string& value);
  std::string* _internal_mutable_email();
  public:

  // @@protoc_insertion_point(class_scope:message.VerifyRequst)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr email_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_message_2eproto;
};
// -------------------------------------------------------------------

class VerifyResponse PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:message.VerifyResponse) */ {
 public:
  inline VerifyResponse() : VerifyResponse(nullptr) {}
  virtual ~VerifyResponse();

  VerifyResponse(const VerifyResponse& from);
  VerifyResponse(VerifyResponse&& from) noexcept
    : VerifyResponse() {
    *this = ::std::move(from);
  }

  inline VerifyResponse& operator=(const VerifyResponse& from) {
    CopyFrom(from);
    return *this;
  }
  inline VerifyResponse& operator=(VerifyResponse&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const VerifyResponse& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const VerifyResponse* internal_default_instance() {
    return reinterpret_cast<const VerifyResponse*>(
               &_VerifyResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(VerifyResponse& a, VerifyResponse& b) {
    a.Swap(&b);
  }
  inline void Swap(VerifyResponse* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(VerifyResponse* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline VerifyResponse* New() const final {
    return CreateMaybeMessage<VerifyResponse>(nullptr);
  }

  VerifyResponse* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<VerifyResponse>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const VerifyResponse& from);
  void MergeFrom(const VerifyResponse& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(VerifyResponse* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "message.VerifyResponse";
  }
  protected:
  explicit VerifyResponse(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_message_2eproto);
    return ::descriptor_table_message_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kEmailFieldNumber = 2,
    kCodeFieldNumber = 3,
    kErrorFieldNumber = 1,
  };
  // string email = 2;
  void clear_email();
  const std::string& email() const;
  void set_email(const std::string& value);
  void set_email(std::string&& value);
  void set_email(const char* value);
  void set_email(const char* value, size_t size);
  std::string* mutable_email();
  std::string* release_email();
  void set_allocated_email(std::string* email);
  private:
  const std::string& _internal_email() const;
  void _internal_set_email(const std::string& value);
  std::string* _internal_mutable_email();
  public:

  // string code = 3;
  void clear_code();
  const std::string& code() const;
  void set_code(const std::string& value);
  void set_code(std::string&& value);
  void set_code(const char* value);
  void set_code(const char* value, size_t size);
  std::string* mutable_code();
  std::string* release_code();
  void set_allocated_code(std::string* code);
  private:
  const std::string& _internal_code() const;
  void _internal_set_code(const std::string& value);
  std::string* _internal_mutable_code();
  public:

  // int32 error = 1;
  void clear_error();
  ::PROTOBUF_NAMESPACE_ID::int32 error() const;
  void set_error(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_error() const;
  void _internal_set_error(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // @@protoc_insertion_point(class_scope:message.VerifyResponse)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr email_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr code_;
  ::PROTOBUF_NAMESPACE_ID::int32 error_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_message_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// VerifyRequst

// string email = 1;
inline void VerifyRequst::clear_email() {
  email_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& VerifyRequst::email() const {
  // @@protoc_insertion_point(field_get:message.VerifyRequst.email)
  return _internal_email();
}
inline void VerifyRequst::set_email(const std::string& value) {
  _internal_set_email(value);
  // @@protoc_insertion_point(field_set:message.VerifyRequst.email)
}
inline std::string* VerifyRequst::mutable_email() {
  // @@protoc_insertion_point(field_mutable:message.VerifyRequst.email)
  return _internal_mutable_email();
}
inline const std::string& VerifyRequst::_internal_email() const {
  return email_.Get();
}
inline void VerifyRequst::_internal_set_email(const std::string& value) {
  
  email_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void VerifyRequst::set_email(std::string&& value) {
  
  email_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:message.VerifyRequst.email)
}
inline void VerifyRequst::set_email(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  email_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:message.VerifyRequst.email)
}
inline void VerifyRequst::set_email(const char* value,
    size_t size) {
  
  email_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:message.VerifyRequst.email)
}
inline std::string* VerifyRequst::_internal_mutable_email() {
  
  return email_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* VerifyRequst::release_email() {
  // @@protoc_insertion_point(field_release:message.VerifyRequst.email)
  return email_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void VerifyRequst::set_allocated_email(std::string* email) {
  if (email != nullptr) {
    
  } else {
    
  }
  email_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), email,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:message.VerifyRequst.email)
}

// -------------------------------------------------------------------

// VerifyResponse

// int32 error = 1;
inline void VerifyResponse::clear_error() {
  error_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 VerifyResponse::_internal_error() const {
  return error_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 VerifyResponse::error() const {
  // @@protoc_insertion_point(field_get:message.VerifyResponse.error)
  return _internal_error();
}
inline void VerifyResponse::_internal_set_error(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  error_ = value;
}
inline void VerifyResponse::set_error(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_error(value);
  // @@protoc_insertion_point(field_set:message.VerifyResponse.error)
}

// string email = 2;
inline void VerifyResponse::clear_email() {
  email_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& VerifyResponse::email() const {
  // @@protoc_insertion_point(field_get:message.VerifyResponse.email)
  return _internal_email();
}
inline void VerifyResponse::set_email(const std::string& value) {
  _internal_set_email(value);
  // @@protoc_insertion_point(field_set:message.VerifyResponse.email)
}
inline std::string* VerifyResponse::mutable_email() {
  // @@protoc_insertion_point(field_mutable:message.VerifyResponse.email)
  return _internal_mutable_email();
}
inline const std::string& VerifyResponse::_internal_email() const {
  return email_.Get();
}
inline void VerifyResponse::_internal_set_email(const std::string& value) {
  
  email_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void VerifyResponse::set_email(std::string&& value) {
  
  email_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:message.VerifyResponse.email)
}
inline void VerifyResponse::set_email(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  email_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:message.VerifyResponse.email)
}
inline void VerifyResponse::set_email(const char* value,
    size_t size) {
  
  email_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:message.VerifyResponse.email)
}
inline std::string* VerifyResponse::_internal_mutable_email() {
  
  return email_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* VerifyResponse::release_email() {
  // @@protoc_insertion_point(field_release:message.VerifyResponse.email)
  return email_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void VerifyResponse::set_allocated_email(std::string* email) {
  if (email != nullptr) {
    
  } else {
    
  }
  email_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), email,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:message.VerifyResponse.email)
}

// string code = 3;
inline void VerifyResponse::clear_code() {
  code_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& VerifyResponse::code() const {
  // @@protoc_insertion_point(field_get:message.VerifyResponse.code)
  return _internal_code();
}
inline void VerifyResponse::set_code(const std::string& value) {
  _internal_set_code(value);
  // @@protoc_insertion_point(field_set:message.VerifyResponse.code)
}
inline std::string* VerifyResponse::mutable_code() {
  // @@protoc_insertion_point(field_mutable:message.VerifyResponse.code)
  return _internal_mutable_code();
}
inline const std::string& VerifyResponse::_internal_code() const {
  return code_.Get();
}
inline void VerifyResponse::_internal_set_code(const std::string& value) {
  
  code_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void VerifyResponse::set_code(std::string&& value) {
  
  code_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:message.VerifyResponse.code)
}
inline void VerifyResponse::set_code(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  code_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:message.VerifyResponse.code)
}
inline void VerifyResponse::set_code(const char* value,
    size_t size) {
  
  code_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:message.VerifyResponse.code)
}
inline std::string* VerifyResponse::_internal_mutable_code() {
  
  return code_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* VerifyResponse::release_code() {
  // @@protoc_insertion_point(field_release:message.VerifyResponse.code)
  return code_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void VerifyResponse::set_allocated_code(std::string* code) {
  if (code != nullptr) {
    
  } else {
    
  }
  code_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), code,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:message.VerifyResponse.code)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace message

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_message_2eproto
