#ifndef MY_ASSERT_H_2323
#define MY_ASSERT_H_2323

//NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_IN_RELEASE 0

#if defined(NDEBUG) && ASSERT_IN_RELEASE
#warning "Assert statements enabled in release build. This will impact performance"
#endif

// From: https://stackoverflow.com/questions/3692954/add-custom-messages-in-assert
#if not defined(NDEBUG) || ASSERT_IN_RELEASE
//NOLINTNEXTLINE
#define MY_ASSERT(Expr, Msg) my_assert_utl_macro_(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
//NOLINTNEXTLINE
#define MY_ASSERT(Expr, Msg) ;
#endif

constexpr void my_assert_utl_macro_(char const* expr_str, bool expr, char const* file, int line, char const* msg)
{
  if (!expr)
  {
    std::cerr << "Assert failed:\t" << msg << "\n"
              << "Expected:\t" << expr_str << "\n"
              << "Source:\t\t" << file << ", line " << line << "\n";
    abort();
  }
}

#endif // MY_ASSERT_H_2323
