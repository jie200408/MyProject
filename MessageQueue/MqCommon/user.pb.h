// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: user.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_user_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_user_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3014000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3014000 < PROTOBUF_MIN_PROTOC_VERSION
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
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_user_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_user_2eproto {
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
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_user_2eproto;
namespace mq {
class userLogin;
class userLoginDefaultTypeInternal;
extern userLoginDefaultTypeInternal _userLogin_default_instance_;
class userLogout;
class userLogoutDefaultTypeInternal;
extern userLogoutDefaultTypeInternal _userLogout_default_instance_;
}  // namespace mq
PROTOBUF_NAMESPACE_OPEN
template<> ::mq::userLogin* Arena::CreateMaybeMessage<::mq::userLogin>(Arena*);
template<> ::mq::userLogout* Arena::CreateMaybeMessage<::mq::userLogout>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace mq {

enum UserType : int {
  RECIVER = 0,
  PUBLISHER = 1,
  ADMIN = 2,
  UserType_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::min(),
  UserType_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::max()
};
bool UserType_IsValid(int value);
constexpr UserType UserType_MIN = RECIVER;
constexpr UserType UserType_MAX = ADMIN;
constexpr int UserType_ARRAYSIZE = UserType_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* UserType_descriptor();
template<typename T>
inline const std::string& UserType_Name(T enum_t_value) {
  static_assert(::std::is_same<T, UserType>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function UserType_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    UserType_descriptor(), enum_t_value);
}
inline bool UserType_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, UserType* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<UserType>(
    UserType_descriptor(), name, value);
}
// ===================================================================

class userLogin PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:mq.userLogin) */ {
 public:
  inline userLogin() : userLogin(nullptr) {}
  virtual ~userLogin();

  userLogin(const userLogin& from);
  userLogin(userLogin&& from) noexcept
    : userLogin() {
    *this = ::std::move(from);
  }

  inline userLogin& operator=(const userLogin& from) {
    CopyFrom(from);
    return *this;
  }
  inline userLogin& operator=(userLogin&& from) noexcept {
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
  static const userLogin& default_instance();

  static inline const userLogin* internal_default_instance() {
    return reinterpret_cast<const userLogin*>(
               &_userLogin_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(userLogin& a, userLogin& b) {
    a.Swap(&b);
  }
  inline void Swap(userLogin* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(userLogin* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline userLogin* New() const final {
    return CreateMaybeMessage<userLogin>(nullptr);
  }

  userLogin* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<userLogin>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const userLogin& from);
  void MergeFrom(const userLogin& from);
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
  void InternalSwap(userLogin* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "mq.userLogin";
  }
  protected:
  explicit userLogin(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_user_2eproto);
    return ::descriptor_table_user_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kRidFieldNumber = 1,
    kCidFieldNumber = 2,
    kUsernameFieldNumber = 3,
    kPasswordFieldNumber = 4,
    kIsRegisterFieldNumber = 5,
    kUserTypeFieldNumber = 6,
  };
  // string rid = 1;
  void clear_rid();
  const std::string& rid() const;
  void set_rid(const std::string& value);
  void set_rid(std::string&& value);
  void set_rid(const char* value);
  void set_rid(const char* value, size_t size);
  std::string* mutable_rid();
  std::string* release_rid();
  void set_allocated_rid(std::string* rid);
  private:
  const std::string& _internal_rid() const;
  void _internal_set_rid(const std::string& value);
  std::string* _internal_mutable_rid();
  public:

  // string cid = 2;
  void clear_cid();
  const std::string& cid() const;
  void set_cid(const std::string& value);
  void set_cid(std::string&& value);
  void set_cid(const char* value);
  void set_cid(const char* value, size_t size);
  std::string* mutable_cid();
  std::string* release_cid();
  void set_allocated_cid(std::string* cid);
  private:
  const std::string& _internal_cid() const;
  void _internal_set_cid(const std::string& value);
  std::string* _internal_mutable_cid();
  public:

  // string username = 3;
  void clear_username();
  const std::string& username() const;
  void set_username(const std::string& value);
  void set_username(std::string&& value);
  void set_username(const char* value);
  void set_username(const char* value, size_t size);
  std::string* mutable_username();
  std::string* release_username();
  void set_allocated_username(std::string* username);
  private:
  const std::string& _internal_username() const;
  void _internal_set_username(const std::string& value);
  std::string* _internal_mutable_username();
  public:

  // string password = 4;
  void clear_password();
  const std::string& password() const;
  void set_password(const std::string& value);
  void set_password(std::string&& value);
  void set_password(const char* value);
  void set_password(const char* value, size_t size);
  std::string* mutable_password();
  std::string* release_password();
  void set_allocated_password(std::string* password);
  private:
  const std::string& _internal_password() const;
  void _internal_set_password(const std::string& value);
  std::string* _internal_mutable_password();
  public:

  // bool isRegister = 5;
  void clear_isregister();
  bool isregister() const;
  void set_isregister(bool value);
  private:
  bool _internal_isregister() const;
  void _internal_set_isregister(bool value);
  public:

  // .mq.UserType user_type = 6;
  void clear_user_type();
  ::mq::UserType user_type() const;
  void set_user_type(::mq::UserType value);
  private:
  ::mq::UserType _internal_user_type() const;
  void _internal_set_user_type(::mq::UserType value);
  public:

  // @@protoc_insertion_point(class_scope:mq.userLogin)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr rid_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr cid_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr username_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr password_;
  bool isregister_;
  int user_type_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_user_2eproto;
};
// -------------------------------------------------------------------

class userLogout PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:mq.userLogout) */ {
 public:
  inline userLogout() : userLogout(nullptr) {}
  virtual ~userLogout();

  userLogout(const userLogout& from);
  userLogout(userLogout&& from) noexcept
    : userLogout() {
    *this = ::std::move(from);
  }

  inline userLogout& operator=(const userLogout& from) {
    CopyFrom(from);
    return *this;
  }
  inline userLogout& operator=(userLogout&& from) noexcept {
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
  static const userLogout& default_instance();

  static inline const userLogout* internal_default_instance() {
    return reinterpret_cast<const userLogout*>(
               &_userLogout_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(userLogout& a, userLogout& b) {
    a.Swap(&b);
  }
  inline void Swap(userLogout* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(userLogout* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline userLogout* New() const final {
    return CreateMaybeMessage<userLogout>(nullptr);
  }

  userLogout* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<userLogout>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const userLogout& from);
  void MergeFrom(const userLogout& from);
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
  void InternalSwap(userLogout* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "mq.userLogout";
  }
  protected:
  explicit userLogout(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_user_2eproto);
    return ::descriptor_table_user_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kRidFieldNumber = 1,
    kCidFieldNumber = 2,
    kUsernameFieldNumber = 3,
    kUserTypeFieldNumber = 4,
  };
  // string rid = 1;
  void clear_rid();
  const std::string& rid() const;
  void set_rid(const std::string& value);
  void set_rid(std::string&& value);
  void set_rid(const char* value);
  void set_rid(const char* value, size_t size);
  std::string* mutable_rid();
  std::string* release_rid();
  void set_allocated_rid(std::string* rid);
  private:
  const std::string& _internal_rid() const;
  void _internal_set_rid(const std::string& value);
  std::string* _internal_mutable_rid();
  public:

  // string cid = 2;
  void clear_cid();
  const std::string& cid() const;
  void set_cid(const std::string& value);
  void set_cid(std::string&& value);
  void set_cid(const char* value);
  void set_cid(const char* value, size_t size);
  std::string* mutable_cid();
  std::string* release_cid();
  void set_allocated_cid(std::string* cid);
  private:
  const std::string& _internal_cid() const;
  void _internal_set_cid(const std::string& value);
  std::string* _internal_mutable_cid();
  public:

  // string username = 3;
  void clear_username();
  const std::string& username() const;
  void set_username(const std::string& value);
  void set_username(std::string&& value);
  void set_username(const char* value);
  void set_username(const char* value, size_t size);
  std::string* mutable_username();
  std::string* release_username();
  void set_allocated_username(std::string* username);
  private:
  const std::string& _internal_username() const;
  void _internal_set_username(const std::string& value);
  std::string* _internal_mutable_username();
  public:

  // .mq.UserType user_type = 4;
  void clear_user_type();
  ::mq::UserType user_type() const;
  void set_user_type(::mq::UserType value);
  private:
  ::mq::UserType _internal_user_type() const;
  void _internal_set_user_type(::mq::UserType value);
  public:

  // @@protoc_insertion_point(class_scope:mq.userLogout)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr rid_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr cid_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr username_;
  int user_type_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_user_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// userLogin

// string rid = 1;
inline void userLogin::clear_rid() {
  rid_.ClearToEmpty();
}
inline const std::string& userLogin::rid() const {
  // @@protoc_insertion_point(field_get:mq.userLogin.rid)
  return _internal_rid();
}
inline void userLogin::set_rid(const std::string& value) {
  _internal_set_rid(value);
  // @@protoc_insertion_point(field_set:mq.userLogin.rid)
}
inline std::string* userLogin::mutable_rid() {
  // @@protoc_insertion_point(field_mutable:mq.userLogin.rid)
  return _internal_mutable_rid();
}
inline const std::string& userLogin::_internal_rid() const {
  return rid_.Get();
}
inline void userLogin::_internal_set_rid(const std::string& value) {
  
  rid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void userLogin::set_rid(std::string&& value) {
  
  rid_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mq.userLogin.rid)
}
inline void userLogin::set_rid(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  rid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:mq.userLogin.rid)
}
inline void userLogin::set_rid(const char* value,
    size_t size) {
  
  rid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mq.userLogin.rid)
}
inline std::string* userLogin::_internal_mutable_rid() {
  
  return rid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* userLogin::release_rid() {
  // @@protoc_insertion_point(field_release:mq.userLogin.rid)
  return rid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void userLogin::set_allocated_rid(std::string* rid) {
  if (rid != nullptr) {
    
  } else {
    
  }
  rid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), rid,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mq.userLogin.rid)
}

// string cid = 2;
inline void userLogin::clear_cid() {
  cid_.ClearToEmpty();
}
inline const std::string& userLogin::cid() const {
  // @@protoc_insertion_point(field_get:mq.userLogin.cid)
  return _internal_cid();
}
inline void userLogin::set_cid(const std::string& value) {
  _internal_set_cid(value);
  // @@protoc_insertion_point(field_set:mq.userLogin.cid)
}
inline std::string* userLogin::mutable_cid() {
  // @@protoc_insertion_point(field_mutable:mq.userLogin.cid)
  return _internal_mutable_cid();
}
inline const std::string& userLogin::_internal_cid() const {
  return cid_.Get();
}
inline void userLogin::_internal_set_cid(const std::string& value) {
  
  cid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void userLogin::set_cid(std::string&& value) {
  
  cid_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mq.userLogin.cid)
}
inline void userLogin::set_cid(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  cid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:mq.userLogin.cid)
}
inline void userLogin::set_cid(const char* value,
    size_t size) {
  
  cid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mq.userLogin.cid)
}
inline std::string* userLogin::_internal_mutable_cid() {
  
  return cid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* userLogin::release_cid() {
  // @@protoc_insertion_point(field_release:mq.userLogin.cid)
  return cid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void userLogin::set_allocated_cid(std::string* cid) {
  if (cid != nullptr) {
    
  } else {
    
  }
  cid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), cid,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mq.userLogin.cid)
}

// string username = 3;
inline void userLogin::clear_username() {
  username_.ClearToEmpty();
}
inline const std::string& userLogin::username() const {
  // @@protoc_insertion_point(field_get:mq.userLogin.username)
  return _internal_username();
}
inline void userLogin::set_username(const std::string& value) {
  _internal_set_username(value);
  // @@protoc_insertion_point(field_set:mq.userLogin.username)
}
inline std::string* userLogin::mutable_username() {
  // @@protoc_insertion_point(field_mutable:mq.userLogin.username)
  return _internal_mutable_username();
}
inline const std::string& userLogin::_internal_username() const {
  return username_.Get();
}
inline void userLogin::_internal_set_username(const std::string& value) {
  
  username_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void userLogin::set_username(std::string&& value) {
  
  username_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mq.userLogin.username)
}
inline void userLogin::set_username(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  username_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:mq.userLogin.username)
}
inline void userLogin::set_username(const char* value,
    size_t size) {
  
  username_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mq.userLogin.username)
}
inline std::string* userLogin::_internal_mutable_username() {
  
  return username_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* userLogin::release_username() {
  // @@protoc_insertion_point(field_release:mq.userLogin.username)
  return username_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void userLogin::set_allocated_username(std::string* username) {
  if (username != nullptr) {
    
  } else {
    
  }
  username_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), username,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mq.userLogin.username)
}

// string password = 4;
inline void userLogin::clear_password() {
  password_.ClearToEmpty();
}
inline const std::string& userLogin::password() const {
  // @@protoc_insertion_point(field_get:mq.userLogin.password)
  return _internal_password();
}
inline void userLogin::set_password(const std::string& value) {
  _internal_set_password(value);
  // @@protoc_insertion_point(field_set:mq.userLogin.password)
}
inline std::string* userLogin::mutable_password() {
  // @@protoc_insertion_point(field_mutable:mq.userLogin.password)
  return _internal_mutable_password();
}
inline const std::string& userLogin::_internal_password() const {
  return password_.Get();
}
inline void userLogin::_internal_set_password(const std::string& value) {
  
  password_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void userLogin::set_password(std::string&& value) {
  
  password_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mq.userLogin.password)
}
inline void userLogin::set_password(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  password_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:mq.userLogin.password)
}
inline void userLogin::set_password(const char* value,
    size_t size) {
  
  password_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mq.userLogin.password)
}
inline std::string* userLogin::_internal_mutable_password() {
  
  return password_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* userLogin::release_password() {
  // @@protoc_insertion_point(field_release:mq.userLogin.password)
  return password_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void userLogin::set_allocated_password(std::string* password) {
  if (password != nullptr) {
    
  } else {
    
  }
  password_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), password,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mq.userLogin.password)
}

// bool isRegister = 5;
inline void userLogin::clear_isregister() {
  isregister_ = false;
}
inline bool userLogin::_internal_isregister() const {
  return isregister_;
}
inline bool userLogin::isregister() const {
  // @@protoc_insertion_point(field_get:mq.userLogin.isRegister)
  return _internal_isregister();
}
inline void userLogin::_internal_set_isregister(bool value) {
  
  isregister_ = value;
}
inline void userLogin::set_isregister(bool value) {
  _internal_set_isregister(value);
  // @@protoc_insertion_point(field_set:mq.userLogin.isRegister)
}

// .mq.UserType user_type = 6;
inline void userLogin::clear_user_type() {
  user_type_ = 0;
}
inline ::mq::UserType userLogin::_internal_user_type() const {
  return static_cast< ::mq::UserType >(user_type_);
}
inline ::mq::UserType userLogin::user_type() const {
  // @@protoc_insertion_point(field_get:mq.userLogin.user_type)
  return _internal_user_type();
}
inline void userLogin::_internal_set_user_type(::mq::UserType value) {
  
  user_type_ = value;
}
inline void userLogin::set_user_type(::mq::UserType value) {
  _internal_set_user_type(value);
  // @@protoc_insertion_point(field_set:mq.userLogin.user_type)
}

// -------------------------------------------------------------------

// userLogout

// string rid = 1;
inline void userLogout::clear_rid() {
  rid_.ClearToEmpty();
}
inline const std::string& userLogout::rid() const {
  // @@protoc_insertion_point(field_get:mq.userLogout.rid)
  return _internal_rid();
}
inline void userLogout::set_rid(const std::string& value) {
  _internal_set_rid(value);
  // @@protoc_insertion_point(field_set:mq.userLogout.rid)
}
inline std::string* userLogout::mutable_rid() {
  // @@protoc_insertion_point(field_mutable:mq.userLogout.rid)
  return _internal_mutable_rid();
}
inline const std::string& userLogout::_internal_rid() const {
  return rid_.Get();
}
inline void userLogout::_internal_set_rid(const std::string& value) {
  
  rid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void userLogout::set_rid(std::string&& value) {
  
  rid_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mq.userLogout.rid)
}
inline void userLogout::set_rid(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  rid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:mq.userLogout.rid)
}
inline void userLogout::set_rid(const char* value,
    size_t size) {
  
  rid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mq.userLogout.rid)
}
inline std::string* userLogout::_internal_mutable_rid() {
  
  return rid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* userLogout::release_rid() {
  // @@protoc_insertion_point(field_release:mq.userLogout.rid)
  return rid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void userLogout::set_allocated_rid(std::string* rid) {
  if (rid != nullptr) {
    
  } else {
    
  }
  rid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), rid,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mq.userLogout.rid)
}

// string cid = 2;
inline void userLogout::clear_cid() {
  cid_.ClearToEmpty();
}
inline const std::string& userLogout::cid() const {
  // @@protoc_insertion_point(field_get:mq.userLogout.cid)
  return _internal_cid();
}
inline void userLogout::set_cid(const std::string& value) {
  _internal_set_cid(value);
  // @@protoc_insertion_point(field_set:mq.userLogout.cid)
}
inline std::string* userLogout::mutable_cid() {
  // @@protoc_insertion_point(field_mutable:mq.userLogout.cid)
  return _internal_mutable_cid();
}
inline const std::string& userLogout::_internal_cid() const {
  return cid_.Get();
}
inline void userLogout::_internal_set_cid(const std::string& value) {
  
  cid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void userLogout::set_cid(std::string&& value) {
  
  cid_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mq.userLogout.cid)
}
inline void userLogout::set_cid(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  cid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:mq.userLogout.cid)
}
inline void userLogout::set_cid(const char* value,
    size_t size) {
  
  cid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mq.userLogout.cid)
}
inline std::string* userLogout::_internal_mutable_cid() {
  
  return cid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* userLogout::release_cid() {
  // @@protoc_insertion_point(field_release:mq.userLogout.cid)
  return cid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void userLogout::set_allocated_cid(std::string* cid) {
  if (cid != nullptr) {
    
  } else {
    
  }
  cid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), cid,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mq.userLogout.cid)
}

// string username = 3;
inline void userLogout::clear_username() {
  username_.ClearToEmpty();
}
inline const std::string& userLogout::username() const {
  // @@protoc_insertion_point(field_get:mq.userLogout.username)
  return _internal_username();
}
inline void userLogout::set_username(const std::string& value) {
  _internal_set_username(value);
  // @@protoc_insertion_point(field_set:mq.userLogout.username)
}
inline std::string* userLogout::mutable_username() {
  // @@protoc_insertion_point(field_mutable:mq.userLogout.username)
  return _internal_mutable_username();
}
inline const std::string& userLogout::_internal_username() const {
  return username_.Get();
}
inline void userLogout::_internal_set_username(const std::string& value) {
  
  username_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArena());
}
inline void userLogout::set_username(std::string&& value) {
  
  username_.Set(
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mq.userLogout.username)
}
inline void userLogout::set_username(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  username_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(value), GetArena());
  // @@protoc_insertion_point(field_set_char:mq.userLogout.username)
}
inline void userLogout::set_username(const char* value,
    size_t size) {
  
  username_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mq.userLogout.username)
}
inline std::string* userLogout::_internal_mutable_username() {
  
  return username_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArena());
}
inline std::string* userLogout::release_username() {
  // @@protoc_insertion_point(field_release:mq.userLogout.username)
  return username_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void userLogout::set_allocated_username(std::string* username) {
  if (username != nullptr) {
    
  } else {
    
  }
  username_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), username,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mq.userLogout.username)
}

// .mq.UserType user_type = 4;
inline void userLogout::clear_user_type() {
  user_type_ = 0;
}
inline ::mq::UserType userLogout::_internal_user_type() const {
  return static_cast< ::mq::UserType >(user_type_);
}
inline ::mq::UserType userLogout::user_type() const {
  // @@protoc_insertion_point(field_get:mq.userLogout.user_type)
  return _internal_user_type();
}
inline void userLogout::_internal_set_user_type(::mq::UserType value) {
  
  user_type_ = value;
}
inline void userLogout::set_user_type(::mq::UserType value) {
  _internal_set_user_type(value);
  // @@protoc_insertion_point(field_set:mq.userLogout.user_type)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace mq

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::mq::UserType> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::mq::UserType>() {
  return ::mq::UserType_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_user_2eproto