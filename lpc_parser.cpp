// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





// "%code requires" blocks.
#line 13 "src/lpc_parser.yy"

    /*
    * SPDX-License-Identifier: MIT
    * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
    */
    #include <string>
    #include <cstdint>

    #ifndef YY_TYPEDEF_YY_SCANNER_T
    #define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
    #endif
    enum class LpcType: uint32_t {
        T_VOID = 0,
        T_DOUBLE,
        T_FLOAT,
        T_INT,
        T_MAPPING,
        T_OBJECT,
        T_STRING,
        T_MIXED = 127,
        T_ARRAY = 1 << 7,
        T_STATIC = 1 << 8,
        T_PRIVATE = 1 << 9,
        T_PUBLIC = 1 << 10,
        T_NOMASK = 1 << 11,
    };

    enum class LpcAssign: int {
        ASSIGN = 0,
        ADD_ASSIGN,
        SUB_ASSIGN,
        MUL_ASSIGN,
        DIV_ASSIGN,
        MOD_ASSIGN,
        LSH_ASSIGN,
        RSH_ASSIGN,
        AND_ASSIGN,
        XOR_ASSIGN,
        OR_ASSIGN,
    };

    enum class LpcOrder: int {
        LESS = 0,
        LESS_EQUAL,
        GREATER,
        GREATER_EQUAL,
    };

#line 93 "lpc_parser.cpp"


# include <cstdlib> // std::abort
# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

// Support move semantics when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

// Support noexcept when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

// Support constexpr when possible.
#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif



#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

namespace yy {
#line 228 "lpc_parser.cpp"




  /// A Bison parser.
  class LpcParser
  {
  public:
#ifdef YYSTYPE
# ifdef __GNUC__
#  pragma GCC message "bison: do not #define YYSTYPE in C++, use %define api.value.type"
# endif
    typedef YYSTYPE value_type;
#else
  /// A buffer to store and retrieve objects.
  ///
  /// Sort of a variant, but does not keep track of the nature
  /// of the stored data, since that knowledge is available
  /// via the current parser state.
  class value_type
  {
  public:
    /// Type of *this.
    typedef value_type self_type;

    /// Empty construction.
    value_type () YY_NOEXCEPT
      : yyraw_ ()
    {}

    /// Construct and fill.
    template <typename T>
    value_type (YY_RVREF (T) t)
    {
      new (yyas_<T> ()) T (YY_MOVE (t));
    }

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    value_type (const self_type&) = delete;
    /// Non copyable.
    self_type& operator= (const self_type&) = delete;
#endif

    /// Destruction, allowed only if empty.
    ~value_type () YY_NOEXCEPT
    {}

# if 201103L <= YY_CPLUSPLUS
    /// Instantiate a \a T in here from \a t.
    template <typename T, typename... U>
    T&
    emplace (U&&... u)
    {
      return *new (yyas_<T> ()) T (std::forward <U>(u)...);
    }
# else
    /// Instantiate an empty \a T in here.
    template <typename T>
    T&
    emplace ()
    {
      return *new (yyas_<T> ()) T ();
    }

    /// Instantiate a \a T in here from \a t.
    template <typename T>
    T&
    emplace (const T& t)
    {
      return *new (yyas_<T> ()) T (t);
    }
# endif

    /// Instantiate an empty \a T in here.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build ()
    {
      return emplace<T> ();
    }

    /// Instantiate a \a T in here from \a t.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build (const T& t)
    {
      return emplace<T> (t);
    }

    /// Accessor to a built \a T.
    template <typename T>
    T&
    as () YY_NOEXCEPT
    {
      return *yyas_<T> ();
    }

    /// Const accessor to a built \a T (for %printer).
    template <typename T>
    const T&
    as () const YY_NOEXCEPT
    {
      return *yyas_<T> ();
    }

    /// Swap the content with \a that, of same type.
    ///
    /// Both variants must be built beforehand, because swapping the actual
    /// data requires reading it (with as()), and this is not possible on
    /// unconstructed variants: it would require some dynamic testing, which
    /// should not be the variant's responsibility.
    /// Swapping between built and (possibly) non-built is done with
    /// self_type::move ().
    template <typename T>
    void
    swap (self_type& that) YY_NOEXCEPT
    {
      std::swap (as<T> (), that.as<T> ());
    }

    /// Move the content of \a that to this.
    ///
    /// Destroys \a that.
    template <typename T>
    void
    move (self_type& that)
    {
# if 201103L <= YY_CPLUSPLUS
      emplace<T> (std::move (that.as<T> ()));
# else
      emplace<T> ();
      swap<T> (that);
# endif
      that.destroy<T> ();
    }

# if 201103L <= YY_CPLUSPLUS
    /// Move the content of \a that to this.
    template <typename T>
    void
    move (self_type&& that)
    {
      emplace<T> (std::move (that.as<T> ()));
      that.destroy<T> ();
    }
#endif

    /// Copy the content of \a that to this.
    template <typename T>
    void
    copy (const self_type& that)
    {
      emplace<T> (that.as<T> ());
    }

    /// Destroy the stored \a T.
    template <typename T>
    void
    destroy ()
    {
      as<T> ().~T ();
    }

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    value_type (const self_type&);
    /// Non copyable.
    self_type& operator= (const self_type&);
#endif

    /// Accessor to raw memory as \a T.
    template <typename T>
    T*
    yyas_ () YY_NOEXCEPT
    {
      void *yyp = yyraw_;
      return static_cast<T*> (yyp);
     }

    /// Const accessor to raw memory as \a T.
    template <typename T>
    const T*
    yyas_ () const YY_NOEXCEPT
    {
      const void *yyp = yyraw_;
      return static_cast<const T*> (yyp);
     }

    /// An auxiliary type to compute the largest semantic type.
    union union_type
    {
      // L_TYPE
      // L_TYPE_MODIFIER
      char dummy1[sizeof (LpcType)];

      // L_REAL_NUMBER
      char dummy2[sizeof (double)];

      // L_INTEGER
      // L_ASSIGN
      // L_ORDER
      char dummy3[sizeof (int)];

      // L_IDENTIFIER
      // L_STRING_LITERAL
      // str_const
      // str_literal
      char dummy4[sizeof (std::string)];

      // storage_or_type
      // typed_storage
      // type_modifier_list
      // function_head
      // opt_star
      char dummy5[sizeof (uint32_t)];
    };

    /// The size of the largest semantic type.
    enum { size = sizeof (union_type) };

    /// A buffer to store semantic values.
    union
    {
      /// Strongest alignment constraints.
      long double yyalign_me_;
      /// A buffer large enough to store any of the semantic values.
      char yyraw_[size];
    };
  };

#endif
    /// Backward compatibility (Bison 3.8).
    typedef value_type semantic_type;


    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const std::string& m)
        : std::runtime_error (m)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;
    };

    /// Token kinds.
    struct token
    {
      enum token_kind_type
      {
        YYEMPTY = -2,
    YYEOF = 0,                     // "end of file"
    YYerror = 256,                 // error
    YYUNDEF = 257,                 // "invalid token"
    L_INHERIT = 258,               // L_INHERIT
    L_IF = 259,                    // L_IF
    L_ELSE = 260,                  // L_ELSE
    L_WHILE = 261,                 // L_WHILE
    L_DO = 262,                    // L_DO
    L_FOR = 263,                   // L_FOR
    L_FOREACH = 264,               // L_FOREACH
    L_IN = 265,                    // L_IN
    L_BREAK = 266,                 // L_BREAK
    L_CONTINUE = 267,              // L_CONTINUE
    L_RETURN = 268,                // L_RETURN
    L_SWITCH = 269,                // L_SWITCH
    L_CASE = 270,                  // L_CASE
    L_DEFAULT = 271,               // L_DEFAULT
    L_TRY = 272,                   // L_TRY
    L_CATCH = 273,                 // L_CATCH
    L_NEW = 274,                   // L_NEW
    LOWER_THAN_ELSE = 275,         // LOWER_THAN_ELSE
    L_IDENTIFIER = 276,            // L_IDENTIFIER
    L_INTEGER = 277,               // L_INTEGER
    L_REAL_NUMBER = 278,           // L_REAL_NUMBER
    L_STRING_LITERAL = 279,        // L_STRING_LITERAL
    L_TYPE = 280,                  // L_TYPE
    L_TYPE_MODIFIER = 281,         // L_TYPE_MODIFIER
    L_ASSIGN = 282,                // L_ASSIGN
    L_ORDER = 283,                 // L_ORDER
    L_LOR = 284,                   // L_LOR
    L_LAND = 285,                  // L_LAND
    L_EQ = 286,                    // L_EQ
    L_NE = 287,                    // L_NE
    L_LSH = 288,                   // L_LSH
    L_RSH = 289,                   // L_RSH
    L_NOT = 290,                   // L_NOT
    L_INC = 291,                   // L_INC
    L_DEC = 292,                   // L_DEC
    L_ELLIPSIS = 293               // L_ELLIPSIS
      };
      /// Backward compatibility alias (Bison 3.6).
      typedef token_kind_type yytokentype;
    };

    /// Token kind, as returned by yylex.
    typedef token::token_kind_type token_kind_type;

    /// Backward compatibility alias (Bison 3.6).
    typedef token_kind_type token_type;

    /// Symbol kinds.
    struct symbol_kind
    {
      enum symbol_kind_type
      {
        YYNTOKENS = 59, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_L_INHERIT = 3,                         // L_INHERIT
        S_L_IF = 4,                              // L_IF
        S_L_ELSE = 5,                            // L_ELSE
        S_L_WHILE = 6,                           // L_WHILE
        S_L_DO = 7,                              // L_DO
        S_L_FOR = 8,                             // L_FOR
        S_L_FOREACH = 9,                         // L_FOREACH
        S_L_IN = 10,                             // L_IN
        S_L_BREAK = 11,                          // L_BREAK
        S_L_CONTINUE = 12,                       // L_CONTINUE
        S_L_RETURN = 13,                         // L_RETURN
        S_L_SWITCH = 14,                         // L_SWITCH
        S_L_CASE = 15,                           // L_CASE
        S_L_DEFAULT = 16,                        // L_DEFAULT
        S_L_TRY = 17,                            // L_TRY
        S_L_CATCH = 18,                          // L_CATCH
        S_L_NEW = 19,                            // L_NEW
        S_LOWER_THAN_ELSE = 20,                  // LOWER_THAN_ELSE
        S_L_IDENTIFIER = 21,                     // L_IDENTIFIER
        S_L_INTEGER = 22,                        // L_INTEGER
        S_L_REAL_NUMBER = 23,                    // L_REAL_NUMBER
        S_L_STRING_LITERAL = 24,                 // L_STRING_LITERAL
        S_L_TYPE = 25,                           // L_TYPE
        S_L_TYPE_MODIFIER = 26,                  // L_TYPE_MODIFIER
        S_L_ASSIGN = 27,                         // L_ASSIGN
        S_L_ORDER = 28,                          // L_ORDER
        S_29_ = 29,                              // '?'
        S_L_LOR = 30,                            // L_LOR
        S_L_LAND = 31,                           // L_LAND
        S_32_ = 32,                              // '|'
        S_33_ = 33,                              // '^'
        S_34_ = 34,                              // '&'
        S_L_EQ = 35,                             // L_EQ
        S_L_NE = 36,                             // L_NE
        S_37_ = 37,                              // '<'
        S_L_LSH = 38,                            // L_LSH
        S_L_RSH = 39,                            // L_RSH
        S_40_ = 40,                              // '+'
        S_41_ = 41,                              // '-'
        S_42_ = 42,                              // '*'
        S_43_ = 43,                              // '%'
        S_44_ = 44,                              // '/'
        S_L_NOT = 45,                            // L_NOT
        S_46_ = 46,                              // '~'
        S_L_INC = 47,                            // L_INC
        S_L_DEC = 48,                            // L_DEC
        S_L_ELLIPSIS = 49,                       // L_ELLIPSIS
        S_50_ = 50,                              // ';'
        S_51_ = 51,                              // '('
        S_52_ = 52,                              // ')'
        S_53_ = 53,                              // ','
        S_54_ = 54,                              // '{'
        S_55_ = 55,                              // '}'
        S_56_ = 56,                              // ':'
        S_57_ = 57,                              // '['
        S_58_ = 58,                              // ']'
        S_YYACCEPT = 59,                         // $accept
        S_program = 60,                          // program
        S_extra_semicolon = 61,                  // extra_semicolon
        S_def = 62,                              // def
        S_63_1 = 63,                             // $@1
        S_64_2 = 64,                             // $@2
        S_storage_or_type = 65,                  // storage_or_type
        S_typed_storage = 66,                    // typed_storage
        S_type_modifier_list = 67,               // type_modifier_list
        S_function_head = 68,                    // function_head
        S_opt_star = 69,                         // opt_star
        S_var_list = 70,                         // var_list
        S_new_var = 71,                          // new_var
        S_inheritance = 72,                      // inheritance
        S_str_const = 73,                        // str_const
        S_integer = 74,                          // integer
        S_real_number = 75,                      // real_number
        S_str_literal = 76,                      // str_literal
        S_opt_parameter_list = 77,               // opt_parameter_list
        S_parameter_list = 78,                   // parameter_list
        S_parameter_decl = 79,                   // parameter_decl
        S_block_or_semicolon = 80,               // block_or_semicolon
        S_block = 81,                            // block
        S_stmt_list = 82,                        // stmt_list
        S_stmt = 83,                             // stmt
        S_comma_expr = 84,                       // comma_expr
        S_expr0 = 85,                            // expr0
        S_lvalue = 86,                           // lvalue
        S_expr4 = 87,                            // expr4
        S_local_decl = 88,                       // local_decl
        S_if_stmt = 89,                          // if_stmt
        S_optional_else_stmt = 90,               // optional_else_stmt
        S_while_stmt = 91,                       // while_stmt
        S_do_stmt = 92,                          // do_stmt
        S_return_stmt = 93,                      // return_stmt
        S_function_call = 94,                    // function_call
        S_opt_arg_list = 95,                     // opt_arg_list
        S_arg_list = 96                          // arg_list
      };
    };

    /// (Internal) symbol kind.
    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    /// The number of tokens.
    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;

    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol () YY_NOEXCEPT
        : value ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value ()
      {
        switch (this->kind ())
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.move< LpcType > (std::move (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (std::move (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (std::move (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (std::move (that.value));
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.move< uint32_t > (std::move (that.value));
        break;

      default:
        break;
    }

      }
#endif

      /// Copy constructor.
      basic_symbol (const basic_symbol& that);

      /// Constructors for typed symbols.
#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t)
        : Base (t)
      {}
#else
      basic_symbol (typename Base::kind_type t)
        : Base (t)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, LpcType&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const LpcType& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, double&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const double& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, int&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const int& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::string&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::string& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, uint32_t&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const uint32_t& v)
        : Base (t)
        , value (v)
      {}
#endif

      /// Destroy the symbol.
      ~basic_symbol ()
      {
        clear ();
      }



      /// Destroy contents, and record that is empty.
      void clear () YY_NOEXCEPT
      {
        // User destructor.
        symbol_kind_type yykind = this->kind ();
        basic_symbol<Base>& yysym = *this;
        (void) yysym;
        switch (yykind)
        {
       default:
          break;
        }

        // Value type destructor.
switch (yykind)
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.template destroy< LpcType > ();
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.template destroy< double > ();
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.template destroy< int > ();
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.template destroy< std::string > ();
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.template destroy< uint32_t > ();
        break;

      default:
        break;
    }

        Base::clear ();
      }

#if YYDEBUG || 0
      /// The user-facing name of this symbol.
      const char *name () const YY_NOEXCEPT
      {
        return LpcParser::symbol_name (this->kind ());
      }
#endif // #if YYDEBUG || 0


      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (basic_symbol& s);

      /// The semantic value.
      value_type value;

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct by_kind
    {
      /// The symbol kind as needed by the constructor.
      typedef token_kind_type kind_type;

      /// Default constructor.
      by_kind () YY_NOEXCEPT;

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      by_kind (by_kind&& that) YY_NOEXCEPT;
#endif

      /// Copy constructor.
      by_kind (const by_kind& that) YY_NOEXCEPT;

      /// Constructor from (external) token numbers.
      by_kind (kind_type t) YY_NOEXCEPT;



      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_kind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// The symbol kind.
      /// \a S_YYEMPTY when empty.
      symbol_kind_type kind_;
    };

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef by_kind by_type;

    /// "External" symbols: returned by the scanner.
    struct symbol_type : basic_symbol<by_kind>
    {
      /// Superclass.
      typedef basic_symbol<by_kind> super_type;

      /// Empty symbol.
      symbol_type () YY_NOEXCEPT {}

      /// Constructor for valueless symbols, and symbols from each type.
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok)
        : super_type (token_kind_type (tok))
#else
      symbol_type (int tok)
        : super_type (token_kind_type (tok))
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, LpcType v)
        : super_type (token_kind_type (tok), std::move (v))
#else
      symbol_type (int tok, const LpcType& v)
        : super_type (token_kind_type (tok), v)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, double v)
        : super_type (token_kind_type (tok), std::move (v))
#else
      symbol_type (int tok, const double& v)
        : super_type (token_kind_type (tok), v)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, int v)
        : super_type (token_kind_type (tok), std::move (v))
#else
      symbol_type (int tok, const int& v)
        : super_type (token_kind_type (tok), v)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, std::string v)
        : super_type (token_kind_type (tok), std::move (v))
#else
      symbol_type (int tok, const std::string& v)
        : super_type (token_kind_type (tok), v)
#endif
      {}
    };

    /// Build a parser object.
    LpcParser (yyscan_t yyscanner_yyarg);
    virtual ~LpcParser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    LpcParser (const LpcParser&) = delete;
    /// Non copyable.
    LpcParser& operator= (const LpcParser&) = delete;
#endif

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param msg    a description of the syntax error.
    virtual void error (const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

#if YYDEBUG || 0
    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static const char *symbol_name (symbol_kind_type yysymbol);
#endif // #if YYDEBUG || 0


    // Implementation of make_symbol for each token kind.
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYEOF ()
      {
        return symbol_type (token::YYEOF);
      }
#else
      static
      symbol_type
      make_YYEOF ()
      {
        return symbol_type (token::YYEOF);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYerror ()
      {
        return symbol_type (token::YYerror);
      }
#else
      static
      symbol_type
      make_YYerror ()
      {
        return symbol_type (token::YYerror);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYUNDEF ()
      {
        return symbol_type (token::YYUNDEF);
      }
#else
      static
      symbol_type
      make_YYUNDEF ()
      {
        return symbol_type (token::YYUNDEF);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_INHERIT ()
      {
        return symbol_type (token::L_INHERIT);
      }
#else
      static
      symbol_type
      make_L_INHERIT ()
      {
        return symbol_type (token::L_INHERIT);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_IF ()
      {
        return symbol_type (token::L_IF);
      }
#else
      static
      symbol_type
      make_L_IF ()
      {
        return symbol_type (token::L_IF);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ELSE ()
      {
        return symbol_type (token::L_ELSE);
      }
#else
      static
      symbol_type
      make_L_ELSE ()
      {
        return symbol_type (token::L_ELSE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_WHILE ()
      {
        return symbol_type (token::L_WHILE);
      }
#else
      static
      symbol_type
      make_L_WHILE ()
      {
        return symbol_type (token::L_WHILE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_DO ()
      {
        return symbol_type (token::L_DO);
      }
#else
      static
      symbol_type
      make_L_DO ()
      {
        return symbol_type (token::L_DO);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_FOR ()
      {
        return symbol_type (token::L_FOR);
      }
#else
      static
      symbol_type
      make_L_FOR ()
      {
        return symbol_type (token::L_FOR);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_FOREACH ()
      {
        return symbol_type (token::L_FOREACH);
      }
#else
      static
      symbol_type
      make_L_FOREACH ()
      {
        return symbol_type (token::L_FOREACH);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_IN ()
      {
        return symbol_type (token::L_IN);
      }
#else
      static
      symbol_type
      make_L_IN ()
      {
        return symbol_type (token::L_IN);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_BREAK ()
      {
        return symbol_type (token::L_BREAK);
      }
#else
      static
      symbol_type
      make_L_BREAK ()
      {
        return symbol_type (token::L_BREAK);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_CONTINUE ()
      {
        return symbol_type (token::L_CONTINUE);
      }
#else
      static
      symbol_type
      make_L_CONTINUE ()
      {
        return symbol_type (token::L_CONTINUE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_RETURN ()
      {
        return symbol_type (token::L_RETURN);
      }
#else
      static
      symbol_type
      make_L_RETURN ()
      {
        return symbol_type (token::L_RETURN);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_SWITCH ()
      {
        return symbol_type (token::L_SWITCH);
      }
#else
      static
      symbol_type
      make_L_SWITCH ()
      {
        return symbol_type (token::L_SWITCH);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_CASE ()
      {
        return symbol_type (token::L_CASE);
      }
#else
      static
      symbol_type
      make_L_CASE ()
      {
        return symbol_type (token::L_CASE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_DEFAULT ()
      {
        return symbol_type (token::L_DEFAULT);
      }
#else
      static
      symbol_type
      make_L_DEFAULT ()
      {
        return symbol_type (token::L_DEFAULT);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_TRY ()
      {
        return symbol_type (token::L_TRY);
      }
#else
      static
      symbol_type
      make_L_TRY ()
      {
        return symbol_type (token::L_TRY);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_CATCH ()
      {
        return symbol_type (token::L_CATCH);
      }
#else
      static
      symbol_type
      make_L_CATCH ()
      {
        return symbol_type (token::L_CATCH);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_NEW ()
      {
        return symbol_type (token::L_NEW);
      }
#else
      static
      symbol_type
      make_L_NEW ()
      {
        return symbol_type (token::L_NEW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LOWER_THAN_ELSE ()
      {
        return symbol_type (token::LOWER_THAN_ELSE);
      }
#else
      static
      symbol_type
      make_LOWER_THAN_ELSE ()
      {
        return symbol_type (token::LOWER_THAN_ELSE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_IDENTIFIER (std::string v)
      {
        return symbol_type (token::L_IDENTIFIER, std::move (v));
      }
#else
      static
      symbol_type
      make_L_IDENTIFIER (const std::string& v)
      {
        return symbol_type (token::L_IDENTIFIER, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_INTEGER (int v)
      {
        return symbol_type (token::L_INTEGER, std::move (v));
      }
#else
      static
      symbol_type
      make_L_INTEGER (const int& v)
      {
        return symbol_type (token::L_INTEGER, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_REAL_NUMBER (double v)
      {
        return symbol_type (token::L_REAL_NUMBER, std::move (v));
      }
#else
      static
      symbol_type
      make_L_REAL_NUMBER (const double& v)
      {
        return symbol_type (token::L_REAL_NUMBER, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_STRING_LITERAL (std::string v)
      {
        return symbol_type (token::L_STRING_LITERAL, std::move (v));
      }
#else
      static
      symbol_type
      make_L_STRING_LITERAL (const std::string& v)
      {
        return symbol_type (token::L_STRING_LITERAL, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_TYPE (LpcType v)
      {
        return symbol_type (token::L_TYPE, std::move (v));
      }
#else
      static
      symbol_type
      make_L_TYPE (const LpcType& v)
      {
        return symbol_type (token::L_TYPE, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_TYPE_MODIFIER (LpcType v)
      {
        return symbol_type (token::L_TYPE_MODIFIER, std::move (v));
      }
#else
      static
      symbol_type
      make_L_TYPE_MODIFIER (const LpcType& v)
      {
        return symbol_type (token::L_TYPE_MODIFIER, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ASSIGN (int v)
      {
        return symbol_type (token::L_ASSIGN, std::move (v));
      }
#else
      static
      symbol_type
      make_L_ASSIGN (const int& v)
      {
        return symbol_type (token::L_ASSIGN, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ORDER (int v)
      {
        return symbol_type (token::L_ORDER, std::move (v));
      }
#else
      static
      symbol_type
      make_L_ORDER (const int& v)
      {
        return symbol_type (token::L_ORDER, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_LOR ()
      {
        return symbol_type (token::L_LOR);
      }
#else
      static
      symbol_type
      make_L_LOR ()
      {
        return symbol_type (token::L_LOR);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_LAND ()
      {
        return symbol_type (token::L_LAND);
      }
#else
      static
      symbol_type
      make_L_LAND ()
      {
        return symbol_type (token::L_LAND);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_EQ ()
      {
        return symbol_type (token::L_EQ);
      }
#else
      static
      symbol_type
      make_L_EQ ()
      {
        return symbol_type (token::L_EQ);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_NE ()
      {
        return symbol_type (token::L_NE);
      }
#else
      static
      symbol_type
      make_L_NE ()
      {
        return symbol_type (token::L_NE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_LSH ()
      {
        return symbol_type (token::L_LSH);
      }
#else
      static
      symbol_type
      make_L_LSH ()
      {
        return symbol_type (token::L_LSH);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_RSH ()
      {
        return symbol_type (token::L_RSH);
      }
#else
      static
      symbol_type
      make_L_RSH ()
      {
        return symbol_type (token::L_RSH);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_NOT ()
      {
        return symbol_type (token::L_NOT);
      }
#else
      static
      symbol_type
      make_L_NOT ()
      {
        return symbol_type (token::L_NOT);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_INC ()
      {
        return symbol_type (token::L_INC);
      }
#else
      static
      symbol_type
      make_L_INC ()
      {
        return symbol_type (token::L_INC);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_DEC ()
      {
        return symbol_type (token::L_DEC);
      }
#else
      static
      symbol_type
      make_L_DEC ()
      {
        return symbol_type (token::L_DEC);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ELLIPSIS ()
      {
        return symbol_type (token::L_ELLIPSIS);
      }
#else
      static
      symbol_type
      make_L_ELLIPSIS ()
      {
        return symbol_type (token::L_ELLIPSIS);
      }
#endif


  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    LpcParser (const LpcParser&);
    /// Non copyable.
    LpcParser& operator= (const LpcParser&);
#endif


    /// Stored state numbers (used for stacks).
    typedef unsigned char state_type;

    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yysym     the nonterminal to push on the stack
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT;

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT;

    static const short yypact_ninf_;
    static const signed char yytable_ninf_;

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_kind_type enum.
    static symbol_kind_type yytranslate_ (int t) YY_NOEXCEPT;

#if YYDEBUG || 0
    /// For a symbol, its name in clear.
    static const char* const yytname_[];
#endif // #if YYDEBUG || 0


    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const short yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const signed char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const short yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const unsigned char yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const short yytable_[];

    static const short yycheck_[];

    // YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
    // state STATE-NUM.
    static const signed char yystos_[];

    // YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.
    static const signed char yyr1_[];

    // YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.
    static const signed char yyr2_[];


#if YYDEBUG
    // YYRLINE[YYN] -- Source line where rule number YYN was defined.
    static const short yyrline_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r) const;
    /// Print the state stack on the debug stream.
    virtual void yy_stack_print_ () const;

    /// Debugging level.
    int yydebug_;
    /// Debug stream.
    std::ostream* yycdebug_;

    /// \brief Display a symbol kind, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Base>
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct by_state
    {
      /// Default constructor.
      by_state () YY_NOEXCEPT;

      /// The symbol kind as needed by the constructor.
      typedef state_type kind_type;

      /// Constructor.
      by_state (kind_type s) YY_NOEXCEPT;

      /// Copy constructor.
      by_state (const by_state& that) YY_NOEXCEPT;

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_state& that);

      /// The symbol kind (corresponding to \a state).
      /// \a symbol_kind::S_YYEMPTY when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// The state number used to denote an empty symbol.
      /// We use the initial state, as it does not have a value.
      enum { empty_state = 0 };

      /// The state.
      /// \a empty when empty.
      state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct stack_symbol_type : basic_symbol<by_state>
    {
      /// Superclass.
      typedef basic_symbol<by_state> super_type;
      /// Construct an empty symbol.
      stack_symbol_type ();
      /// Move or copy construction.
      stack_symbol_type (YY_RVREF (stack_symbol_type) that);
      /// Steal the contents from \a sym to build this.
      stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
      /// Assignment, needed by push_back by some old implementations.
      /// Moves the contents of that.
      stack_symbol_type& operator= (stack_symbol_type& that);

      /// Assignment, needed by push_back by other implementations.
      /// Needed by some other old implementations.
      stack_symbol_type& operator= (const stack_symbol_type& that);
#endif
    };

    /// A stack with random access from its top.
    template <typename T, typename S = std::vector<T> >
    class stack
    {
    public:
      // Hide our reversed order.
      typedef typename S::iterator iterator;
      typedef typename S::const_iterator const_iterator;
      typedef typename S::size_type size_type;
      typedef typename std::ptrdiff_t index_type;

      stack (size_type n = 200) YY_NOEXCEPT
        : seq_ (n)
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Non copyable.
      stack (const stack&) = delete;
      /// Non copyable.
      stack& operator= (const stack&) = delete;
#endif

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      const T&
      operator[] (index_type i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      T&
      operator[] (index_type i)
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Steal the contents of \a t.
      ///
      /// Close to move-semantics.
      void
      push (YY_MOVE_REF (T) t)
      {
        seq_.push_back (T ());
        operator[] (0).move (t);
      }

      /// Pop elements from the stack.
      void
      pop (std::ptrdiff_t n = 1) YY_NOEXCEPT
      {
        for (; 0 < n; --n)
          seq_.pop_back ();
      }

      /// Pop all elements from the stack.
      void
      clear () YY_NOEXCEPT
      {
        seq_.clear ();
      }

      /// Number of elements on the stack.
      index_type
      size () const YY_NOEXCEPT
      {
        return index_type (seq_.size ());
      }

      /// Iterator on top of the stack (going downwards).
      const_iterator
      begin () const YY_NOEXCEPT
      {
        return seq_.begin ();
      }

      /// Bottom of the stack.
      const_iterator
      end () const YY_NOEXCEPT
      {
        return seq_.end ();
      }

      /// Present a slice of the top of a stack.
      class slice
      {
      public:
        slice (const stack& stack, index_type range) YY_NOEXCEPT
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (index_type i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        index_type range_;
      };

    private:
#if YY_CPLUSPLUS < 201103L
      /// Non copyable.
      stack (const stack&);
      /// Non copyable.
      stack& operator= (const stack&);
#endif
      /// The wrapped container.
      S seq_;
    };


    /// Stack type.
    typedef stack<stack_symbol_type> stack_type;

    /// The stack.
    stack_type yystack_;

    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param sym  the symbol
    /// \warning the contents of \a s.value is stolen.
    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1) YY_NOEXCEPT;

    /// Constants.
    enum
    {
      yylast_ = 359,     ///< Last index in yytable_.
      yynnts_ = 38,  ///< Number of nonterminal symbols.
      yyfinal_ = 2 ///< Termination state number.
    };


    // User arguments.
    yyscan_t yyscanner;

  };

  LpcParser::symbol_kind_type
  LpcParser::yytranslate_ (int t) YY_NOEXCEPT
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    43,    34,     2,
      51,    52,    42,    40,    53,    41,     2,    44,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    56,    50,
      37,     2,     2,    29,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    57,     2,    58,    33,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    54,    32,    55,    46,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    30,    31,    35,    36,    38,    39,
      45,    47,    48,    49
    };
    // Last valid token kind.
    const int code_max = 293;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

  // basic_symbol.
  template <typename Base>
  LpcParser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value ()
  {
    switch (this->kind ())
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.copy< LpcType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.copy< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.copy< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.copy< uint32_t > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }




  template <typename Base>
  LpcParser::symbol_kind_type
  LpcParser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  LpcParser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  LpcParser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.move< LpcType > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.move< uint32_t > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

  }

  // by_kind.
  LpcParser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  LpcParser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  LpcParser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  LpcParser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  LpcParser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  LpcParser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  LpcParser::symbol_kind_type
  LpcParser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  LpcParser::symbol_kind_type
  LpcParser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


} // yy
#line 2126 "lpc_parser.cpp"


// "%code provides" blocks.
#line 9 "src/lpc_parser.yy"

    yy::LpcParser::symbol_type yylex (yyscan_t yyscanner);

#line 2134 "lpc_parser.cpp"






#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif



// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 2211 "lpc_parser.cpp"

  /// Build a parser object.
  LpcParser::LpcParser (yyscan_t yyscanner_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yyscanner (yyscanner_yyarg)
  {}

  LpcParser::~LpcParser ()
  {}

  LpcParser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  LpcParser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  LpcParser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  LpcParser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  LpcParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  LpcParser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  LpcParser::symbol_kind_type
  LpcParser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  LpcParser::stack_symbol_type::stack_symbol_type ()
  {}

  LpcParser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.YY_MOVE_OR_COPY< LpcType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.YY_MOVE_OR_COPY< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.YY_MOVE_OR_COPY< uint32_t > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  LpcParser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s)
  {
    switch (that.kind ())
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.move< LpcType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.move< uint32_t > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  LpcParser::stack_symbol_type&
  LpcParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.copy< LpcType > (that.value);
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.copy< double > (that.value);
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.copy< int > (that.value);
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.copy< uint32_t > (that.value);
        break;

      default:
        break;
    }

    return *this;
  }

  LpcParser::stack_symbol_type&
  LpcParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        value.move< LpcType > (that.value);
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (that.value);
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (that.value);
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        value.move< uint32_t > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  LpcParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  LpcParser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " (";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  LpcParser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  LpcParser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  LpcParser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  LpcParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  LpcParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  LpcParser::debug_level_type
  LpcParser::debug_level () const
  {
    return yydebug_;
  }

  void
  LpcParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  LpcParser::state_type
  LpcParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  LpcParser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  LpcParser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  LpcParser::operator() ()
  {
    return parse ();
  }

  int
  LpcParser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (yyscanner));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
        yylhs.value.emplace< LpcType > ();
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        yylhs.value.emplace< double > ();
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        yylhs.value.emplace< int > ();
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_str_const: // str_const
      case symbol_kind::S_str_literal: // str_literal
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_opt_star: // opt_star
        yylhs.value.emplace< uint32_t > ();
        break;

      default:
        break;
    }



      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 5: // extra_semicolon: ';'
#line 118 "src/lpc_parser.yy"
          { /* ignore extra semicolon(s) between definitions */ }
#line 2743 "lpc_parser.cpp"
    break;

  case 6: // $@1: %empty
#line 123 "src/lpc_parser.yy"
        {
            uint32_t combined_type = yystack_[0].value.as < uint32_t > ();
        }
#line 2751 "lpc_parser.cpp"
    break;

  case 7: // $@2: %empty
#line 127 "src/lpc_parser.yy"
        {
            /* handle function parameters */
        }
#line 2759 "lpc_parser.cpp"
    break;

  case 8: // def: function_head $@1 '(' opt_parameter_list ')' $@2 block_or_semicolon
#line 131 "src/lpc_parser.yy"
        {
            /* handle forward declaration or definition for the function */
        }
#line 2767 "lpc_parser.cpp"
    break;

  case 9: // def: storage_or_type var_list ';'
#line 134 "src/lpc_parser.yy"
                                   { /* variable declarations */ }
#line 2773 "lpc_parser.cpp"
    break;

  case 11: // storage_or_type: type_modifier_list
#line 140 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = yystack_[0].value.as < uint32_t > () | static_cast<uint32_t>(LpcType::T_MIXED); }
#line 2779 "lpc_parser.cpp"
    break;

  case 12: // storage_or_type: typed_storage
#line 142 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = yystack_[0].value.as < uint32_t > (); }
#line 2785 "lpc_parser.cpp"
    break;

  case 13: // typed_storage: L_TYPE
#line 147 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = static_cast<uint32_t>(yystack_[0].value.as < LpcType > ()); }
#line 2791 "lpc_parser.cpp"
    break;

  case 14: // typed_storage: type_modifier_list L_TYPE
#line 149 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = yystack_[1].value.as < uint32_t > () | static_cast<uint32_t>(yystack_[0].value.as < LpcType > ()); }
#line 2797 "lpc_parser.cpp"
    break;

  case 15: // typed_storage: typed_storage L_TYPE_MODIFIER
#line 151 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = yystack_[1].value.as < uint32_t > () | static_cast<uint32_t>(yystack_[0].value.as < LpcType > ()); }
#line 2803 "lpc_parser.cpp"
    break;

  case 16: // type_modifier_list: L_TYPE_MODIFIER
#line 156 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = static_cast<uint32_t>(yystack_[0].value.as < LpcType > ()); }
#line 2809 "lpc_parser.cpp"
    break;

  case 17: // type_modifier_list: type_modifier_list L_TYPE_MODIFIER
#line 158 "src/lpc_parser.yy"
        {
            /* combine the type modifiers using bitwise OR */
            yylhs.value.as < uint32_t > () = yystack_[1].value.as < uint32_t > () | static_cast<uint32_t>(yystack_[0].value.as < LpcType > ());
        }
#line 2818 "lpc_parser.cpp"
    break;

  case 18: // function_head: storage_or_type opt_star L_IDENTIFIER
#line 166 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = yystack_[2].value.as < uint32_t > () | yystack_[1].value.as < uint32_t > (); }
#line 2824 "lpc_parser.cpp"
    break;

  case 19: // function_head: opt_star L_IDENTIFIER
#line 168 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = static_cast<uint32_t>(LpcType::T_MIXED) | yystack_[1].value.as < uint32_t > (); }
#line 2830 "lpc_parser.cpp"
    break;

  case 20: // opt_star: %empty
#line 173 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = 0; }
#line 2836 "lpc_parser.cpp"
    break;

  case 21: // opt_star: '*'
#line 175 "src/lpc_parser.yy"
        { yylhs.value.as < uint32_t > () = static_cast<uint32_t>(LpcType::T_ARRAY); }
#line 2842 "lpc_parser.cpp"
    break;

  case 28: // str_const: L_STRING_LITERAL
#line 195 "src/lpc_parser.yy"
        {
            yylhs.value.as < std::string > () = yystack_[0].value.as < std::string > ();
        }
#line 2850 "lpc_parser.cpp"
    break;

  case 29: // str_const: '(' str_const ')'
#line 199 "src/lpc_parser.yy"
        {
            /* handle parentheses around string constants */
            yylhs.value.as < std::string > () = yystack_[1].value.as < std::string > ();
        }
#line 2859 "lpc_parser.cpp"
    break;

  case 30: // str_const: str_const L_STRING_LITERAL
#line 204 "src/lpc_parser.yy"
        {
            /* append the string literal to the existing string */
            yylhs.value.as < std::string > () += yystack_[0].value.as < std::string > ();
        }
#line 2868 "lpc_parser.cpp"
    break;

  case 33: // str_literal: L_STRING_LITERAL
#line 219 "src/lpc_parser.yy"
      { yylhs.value.as < std::string > () = yystack_[0].value.as < std::string > (); }
#line 2874 "lpc_parser.cpp"
    break;

  case 34: // str_literal: str_literal L_STRING_LITERAL
#line 220 "src/lpc_parser.yy"
      { yylhs.value.as < std::string > () = yystack_[1].value.as < std::string > (); }
#line 2880 "lpc_parser.cpp"
    break;

  case 41: // parameter_decl: L_TYPE opt_star
#line 236 "src/lpc_parser.yy"
                      { /* anonymous parameter */ }
#line 2886 "lpc_parser.cpp"
    break;

  case 42: // parameter_decl: L_IDENTIFIER
#line 237 "src/lpc_parser.yy"
                   { /* implicit mixed type parameter */ }
#line 2892 "lpc_parser.cpp"
    break;


#line 2896 "lpc_parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        std::string msg = YY_("syntax error");
        error (YY_MOVE (msg));
      }


    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;


      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  LpcParser::error (const syntax_error& yyexc)
  {
    error (yyexc.what ());
  }

#if YYDEBUG || 0
  const char *
  LpcParser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytname_[yysymbol];
  }
#endif // #if YYDEBUG || 0









  const short LpcParser::yypact_ninf_ = -139;

  const signed char LpcParser::yytable_ninf_ = -90;

  const short
  LpcParser::yypact_[] =
  {
    -139,    37,  -139,   -20,  -139,  -139,  -139,   -23,    24,    45,
       4,  -139,    61,  -139,  -139,   -20,    -9,  -139,  -139,    77,
     -40,  -139,  -139,   -20,  -139,  -139,    35,  -139,   -13,  -139,
    -139,    42,  -139,    24,    41,    -2,  -139,   177,    79,  -139,
    -139,  -139,    24,    51,   -25,  -139,    74,  -139,  -139,  -139,
     177,   177,   177,   -15,   -15,   177,  -139,  -139,   108,   230,
      65,   -22,  -139,   109,   114,  -139,  -139,    -2,   177,  -139,
    -139,  -139,  -139,    86,  -139,   -31,   230,  -139,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,  -139,  -139,   177,  -139,
      14,  -139,   230,    88,    91,  -139,   177,   111,   201,   247,
     264,   281,   298,   167,   315,   315,   111,    33,    33,    72,
      72,  -139,  -139,  -139,   230,   -44,  -139,  -139,  -139,  -139,
    -139,   177,   230,   177,  -139,    83,   230,   230,    94,   112,
     135,   115,   116,   146,  -139,  -139,    24,    59,  -139,  -139,
      17,  -139,  -139,  -139,  -139,  -139,   177,   177,   156,  -139,
    -139,  -139,    28,    30,  -139,    58,    66,   113,  -139,  -139,
     135,   135,   177,   166,  -139,    68,   135,  -139,   122,  -139,
    -139
  };

  const signed char
  LpcParser::yydefact_[] =
  {
       3,    20,     1,     0,    13,    16,    21,     4,    20,    12,
      11,     6,     0,    10,    28,     0,     0,     5,     2,     0,
       0,    22,    15,     0,    14,    17,     0,    19,     0,    30,
      26,    24,     9,    20,     0,    35,    29,     0,     0,    23,
      27,    42,    20,     0,    36,    37,    91,    31,    32,    33,
       0,     0,     0,     0,     0,     0,    86,    87,    88,    25,
       0,    85,    92,    24,    41,     7,    39,     0,   103,    84,
      82,    83,    78,    89,    79,     0,    58,    34,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,    81,     0,    40,
       0,    38,   105,     0,   104,    90,     0,    69,     0,    62,
      63,    64,    65,    66,    67,    68,    70,    71,    72,    73,
      74,    75,    77,    76,    60,     0,    44,    46,     8,    43,
     102,     0,    59,     0,    93,     0,   106,    61,     0,     0,
       0,     0,     0,     0,    55,    45,    20,    11,    54,    47,
       0,    49,    50,    51,    52,    53,     0,     0,     0,    56,
      57,   100,     0,     0,    48,     0,     0,     0,   101,    94,
       0,     0,     0,    96,    98,     0,     0,    95,     0,    97,
      99
  };

  const short
  LpcParser::yypgoto_[] =
  {
    -139,  -139,  -139,  -139,  -139,  -139,   172,  -139,   173,  -139,
       0,    29,   144,  -139,    -3,  -139,  -139,  -139,  -139,  -139,
     117,  -139,    78,  -139,  -138,   -55,   -34,    69,    73,  -139,
    -139,  -139,  -139,  -139,  -139,  -139,  -139,  -139
  };

  const unsigned char
  LpcParser::yydefgoto_[] =
  {
       0,     1,    18,     7,    26,   100,   146,     9,   147,    11,
      38,    20,    21,    13,    16,    56,    57,    58,    43,    44,
      45,   128,   148,   135,   149,   150,    76,    60,    61,   151,
     152,   177,   153,   154,   155,    62,   103,   104
  };

  const short
  LpcParser::yytable_[] =
  {
      75,    12,   158,    59,    14,   -89,    46,    23,    19,   106,
      32,    29,    28,    33,   134,    29,    69,    70,    71,    41,
      34,   105,   106,    42,    66,   -89,   -89,    17,    67,    24,
      25,    15,   173,   174,   102,    98,    55,     2,   179,    36,
       3,    30,    64,   125,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,     4,     5,   126,    29,     6,   164,   127,    37,
     106,    22,   132,    90,    91,    92,    93,    94,   168,     6,
     169,   106,    27,    33,    24,    25,    35,   138,   162,   139,
     140,    40,    95,   -18,   141,   142,   143,   136,    31,   137,
      63,   165,   166,    65,    46,    47,    48,    49,     4,     5,
     170,   106,    96,    97,    92,    93,    94,   175,   171,   106,
     178,   106,    72,    74,    50,    68,    73,    73,    51,    52,
      53,    54,    77,   144,    55,    99,    37,   127,   145,   138,
     130,   139,   140,    98,   131,   156,   141,   142,   143,    88,
      89,    90,    91,    92,    93,    94,    46,    47,    48,    49,
       4,     5,   167,   157,   172,   159,   160,    46,    47,    48,
      49,   176,   180,     8,    10,   163,    50,    39,   129,     0,
      51,    52,    53,    54,   101,   144,    55,    50,     0,   127,
       0,    51,    52,    53,    54,    78,   161,    55,    46,    47,
      48,    49,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,    51,    52,    53,    54,     0,     0,    55,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   133,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    78,     0,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    78,     0,     0,     0,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    78,
       0,     0,     0,     0,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    78,     0,     0,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    78,     0,     0,     0,     0,     0,     0,
       0,     0,    87,    88,    89,    90,    91,    92,    93,    94
  };

  const short
  LpcParser::yycheck_[] =
  {
      55,     1,   140,    37,    24,    27,    21,     3,     8,    53,
      50,    24,    15,    53,    58,    24,    50,    51,    52,    21,
      23,    52,    53,    25,    49,    47,    48,    50,    53,    25,
      26,    51,   170,   171,    68,    57,    51,     0,   176,    52,
       3,    50,    42,    98,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    25,    26,    50,    24,    42,    50,    54,    27,
      53,    26,   106,    40,    41,    42,    43,    44,    50,    42,
      50,    53,    21,    53,    25,    26,    51,     4,   143,     6,
       7,    50,    27,    51,    11,    12,    13,   131,    21,   133,
      21,   156,   157,    52,    21,    22,    23,    24,    25,    26,
      52,    53,    47,    48,    42,    43,    44,   172,    52,    53,
      52,    53,    53,    54,    41,    51,    53,    54,    45,    46,
      47,    48,    24,    50,    51,    21,    27,    54,    55,     4,
      52,     6,     7,    57,    53,    51,    11,    12,    13,    38,
      39,    40,    41,    42,    43,    44,    21,    22,    23,    24,
      25,    26,     6,    51,    51,    50,    50,    21,    22,    23,
      24,     5,    50,     1,     1,   146,    41,    33,   100,    -1,
      45,    46,    47,    48,    67,    50,    51,    41,    -1,    54,
      -1,    45,    46,    47,    48,    28,    50,    51,    21,    22,
      23,    24,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    -1,    45,    46,    47,    48,    -1,    -1,    51,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    28,    -1,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    28,    -1,    -1,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    28,
      -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    28,    -1,    -1,    -1,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44
  };

  const signed char
  LpcParser::yystos_[] =
  {
       0,    60,     0,     3,    25,    26,    42,    62,    65,    66,
      67,    68,    69,    72,    24,    51,    73,    50,    61,    69,
      70,    71,    26,     3,    25,    26,    63,    21,    73,    24,
      50,    21,    50,    53,    73,    51,    52,    27,    69,    71,
      50,    21,    25,    77,    78,    79,    21,    22,    23,    24,
      41,    45,    46,    47,    48,    51,    74,    75,    76,    85,
      86,    87,    94,    21,    69,    52,    49,    53,    51,    85,
      85,    85,    86,    87,    86,    84,    85,    24,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    27,    47,    48,    57,    21,
      64,    79,    85,    95,    96,    52,    53,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    84,    50,    54,    80,    81,
      52,    53,    85,    56,    58,    82,    85,    85,     4,     6,
       7,    11,    12,    13,    50,    55,    65,    67,    81,    83,
      84,    88,    89,    91,    92,    93,    51,    51,    83,    50,
      50,    50,    84,    70,    50,    84,    84,     6,    50,    50,
      52,    52,    51,    83,    83,    84,     5,    90,    52,    83,
      50
  };

  const signed char
  LpcParser::yyr1_[] =
  {
       0,    59,    60,    60,    61,    61,    63,    64,    62,    62,
      62,    65,    65,    66,    66,    66,    67,    67,    68,    68,
      69,    69,    70,    70,    71,    71,    72,    72,    73,    73,
      73,    74,    75,    76,    76,    77,    77,    78,    78,    78,
      79,    79,    79,    80,    80,    81,    82,    82,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    84,    84,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    86,
      87,    87,    87,    87,    88,    89,    90,    90,    91,    92,
      93,    93,    94,    95,    95,    96,    96
  };

  const signed char
  LpcParser::yyr2_[] =
  {
       0,     2,     3,     0,     0,     1,     0,     0,     7,     3,
       1,     1,     1,     1,     2,     2,     1,     2,     3,     2,
       0,     1,     1,     3,     2,     4,     3,     4,     1,     3,
       2,     1,     1,     1,     2,     0,     1,     1,     3,     2,
       3,     2,     1,     1,     1,     3,     0,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     3,
       3,     5,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     1,     1,
       3,     1,     1,     4,     3,     6,     0,     2,     5,     7,
       2,     3,     4,     0,     1,     1,     3
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const LpcParser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "L_INHERIT", "L_IF",
  "L_ELSE", "L_WHILE", "L_DO", "L_FOR", "L_FOREACH", "L_IN", "L_BREAK",
  "L_CONTINUE", "L_RETURN", "L_SWITCH", "L_CASE", "L_DEFAULT", "L_TRY",
  "L_CATCH", "L_NEW", "LOWER_THAN_ELSE", "L_IDENTIFIER", "L_INTEGER",
  "L_REAL_NUMBER", "L_STRING_LITERAL", "L_TYPE", "L_TYPE_MODIFIER",
  "L_ASSIGN", "L_ORDER", "'?'", "L_LOR", "L_LAND", "'|'", "'^'", "'&'",
  "L_EQ", "L_NE", "'<'", "L_LSH", "L_RSH", "'+'", "'-'", "'*'", "'%'",
  "'/'", "L_NOT", "'~'", "L_INC", "L_DEC", "L_ELLIPSIS", "';'", "'('",
  "')'", "','", "'{'", "'}'", "':'", "'['", "']'", "$accept", "program",
  "extra_semicolon", "def", "$@1", "$@2", "storage_or_type",
  "typed_storage", "type_modifier_list", "function_head", "opt_star",
  "var_list", "new_var", "inheritance", "str_const", "integer",
  "real_number", "str_literal", "opt_parameter_list", "parameter_list",
  "parameter_decl", "block_or_semicolon", "block", "stmt_list", "stmt",
  "comma_expr", "expr0", "lvalue", "expr4", "local_decl", "if_stmt",
  "optional_else_stmt", "while_stmt", "do_stmt", "return_stmt",
  "function_call", "opt_arg_list", "arg_list", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  LpcParser::yyrline_[] =
  {
       0,   112,   112,   113,   117,   118,   123,   127,   122,   134,
     135,   139,   141,   146,   148,   150,   155,   157,   165,   167,
     173,   174,   179,   180,   184,   185,   189,   190,   194,   198,
     203,   211,   215,   219,   220,   224,   225,   229,   230,   231,
     235,   236,   237,   241,   242,   246,   250,   251,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   268,   269,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   305,
     309,   310,   311,   312,   316,   320,   324,   325,   329,   333,
     337,   338,   342,   346,   347,   351,   352
  };

  void
  LpcParser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  LpcParser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 3367 "lpc_parser.cpp"

#line 355 "src/lpc_parser.yy"


// User subroutines section
void yy::LpcParser::error(const std::string &msg) {
    // Handle parse errors here
    throw std::runtime_error(msg);
}
