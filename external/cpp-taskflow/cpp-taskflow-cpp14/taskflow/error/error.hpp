#pragma once

#include <iostream>
#include <sstream>
#include <exception>
#include <system_error>

namespace tf {

/**
@struct Error 

@brief The error category of taskflow.
*/
struct Error : public std::error_category {
  
  /**
  @enum Code 
  @brief Error code definition.
  */
  enum Code : int {
    SUCCESS = 0,
    TASKFLOW,
    EXECUTOR
  };
  
  /**
  @brief returns the name of the taskflow error category
  */
  inline const char* name() const noexcept override final;

  /**
  @brief acquires the singleton instance of the taskflow error category
  */
  inline static const std::error_category& get();
  
  /**
  @brief query the human-readable string of each error code
  */
  inline std::string message(int) const override final;
};

// Function: name
inline const char* Error::name() const noexcept {
  return "Taskflow error";
}

// Function: get 
inline const std::error_category& Error::get() {
  static Error instance;
  return instance;
}

// Function: message
inline std::string Error::message(int code) const {
  auto ec = static_cast<Error::Code>(code); 
  switch(ec) {
    case SUCCESS:
      return "success";
    break;

    case TASKFLOW:
      return "taskflow error";
    break;

    case EXECUTOR:
      return "executor error";
    break;

    default:
      return "unknown";
    break;
  };
}

// Function: make_error_code
// Argument dependent lookup.
inline std::error_code make_error_code(Error::Code e) {
  return std::error_code(static_cast<int>(e), Error::get());
}

}  // end of namespace tf ----------------------------------------------------

// Register for implicit conversion  
namespace std {
  template <>
  struct is_error_code_enum<tf::Error::Code> : true_type {};
}

// ----------------------------------------------------------------------------

namespace tf {

// Procedure: throw_se
// Throws the system error under a given error code.
template <typename T, typename... ArgsT>
void throw_se(const char* fname, const size_t line, Error::Code c, T&& t, ArgsT&&... args) {
  std::ostringstream oss;
  oss << "[" << fname << ":" << line << "] ";
  oss << std::forward<T>(t);
  // Parameter pack 
  // https://stackoverflow.com/questions/27375089/what-is-the-easiest-way-to-print-a-variadic-parameter-pack-using-stdostream
  using expander = int[];
  (void)expander{0, (void(oss << std::forward<ArgsT>(args)), 0)...};

  // C++17
  //(oss << ... << args);

  throw std::system_error(c, oss.str());
}

}  // ------------------------------------------------------------------------

#define TF_THROW(...) tf::throw_se(__FILE__, __LINE__, __VA_ARGS__);

