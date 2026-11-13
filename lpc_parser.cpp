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

// "%code top" blocks.
#line 12 "src/lpc_parser.yy"

    #include "config.hpp"

#line 43 "lpc_parser.cpp"




// "%code requires" blocks.
#line 16 "src/lpc_parser.yy"

    /*
    * SPDX-License-Identifier: MIT
    * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
    */
    #include <cstdint>
    #include <stdexcept>
    #include <string>
    #include <memory>

    #include "vm_opcodes.hpp"
    #include "symbol_table.hpp"
    #include "ast_node.hpp"
    #include "const_pool.hpp"
    #include "lpc_compiler.hpp"

    // typedef yyscan_t for the reentrant lexer handle
    #ifndef YY_TYPEDEF_YY_SCANNER_T
    #define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
    #endif

#line 72 "lpc_parser.cpp"


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
#line 207 "lpc_parser.cpp"


  /// A point in a source file.
  class position
  {
  public:
    /// Type for file name.
    typedef const std::string filename_type;
    /// Type for line and column numbers.
    typedef int counter_type;

    /// Construct a position.
    explicit position (filename_type* f = YY_NULLPTR,
                       counter_type l = 1,
                       counter_type c = 1)
      : filename (f)
      , line (l)
      , column (c)
    {}


    /// Initialization.
    void initialize (filename_type* fn = YY_NULLPTR,
                     counter_type l = 1,
                     counter_type c = 1)
    {
      filename = fn;
      line = l;
      column = c;
    }

    /** \name Line and Column related manipulators
     ** \{ */
    /// (line related) Advance to the COUNT next lines.
    void lines (counter_type count = 1)
    {
      if (count)
        {
          column = 1;
          line = add_ (line, count, 1);
        }
    }

    /// (column related) Advance to the COUNT next columns.
    void columns (counter_type count = 1)
    {
      column = add_ (column, count, 1);
    }
    /** \} */

    /// File name to which this position refers.
    filename_type* filename;
    /// Current line number.
    counter_type line;
    /// Current column number.
    counter_type column;

  private:
    /// Compute max (min, lhs+rhs).
    static counter_type add_ (counter_type lhs, counter_type rhs, counter_type min)
    {
      return lhs + rhs < min ? min : lhs + rhs;
    }
  };

  /// Add \a width columns, in place.
  inline position&
  operator+= (position& res, position::counter_type width)
  {
    res.columns (width);
    return res;
  }

  /// Add \a width columns.
  inline position
  operator+ (position res, position::counter_type width)
  {
    return res += width;
  }

  /// Subtract \a width columns, in place.
  inline position&
  operator-= (position& res, position::counter_type width)
  {
    return res += -width;
  }

  /// Subtract \a width columns.
  inline position
  operator- (position res, position::counter_type width)
  {
    return res -= width;
  }

  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param pos a reference to the position to redirect
   */
  template <typename YYChar>
  std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const position& pos)
  {
    if (pos.filename)
      ostr << *pos.filename << ':';
    return ostr << pos.line << '.' << pos.column;
  }

  /// Two points in a source file.
  class location
  {
  public:
    /// Type for file name.
    typedef position::filename_type filename_type;
    /// Type for line and column numbers.
    typedef position::counter_type counter_type;

    /// Construct a location from \a b to \a e.
    location (const position& b, const position& e)
      : begin (b)
      , end (e)
    {}

    /// Construct a 0-width location in \a p.
    explicit location (const position& p = position ())
      : begin (p)
      , end (p)
    {}

    /// Construct a 0-width location in \a f, \a l, \a c.
    explicit location (filename_type* f,
                       counter_type l = 1,
                       counter_type c = 1)
      : begin (f, l, c)
      , end (f, l, c)
    {}


    /// Initialization.
    void initialize (filename_type* f = YY_NULLPTR,
                     counter_type l = 1,
                     counter_type c = 1)
    {
      begin.initialize (f, l, c);
      end = begin;
    }

    /** \name Line and Column related manipulators
     ** \{ */
  public:
    /// Reset initial location to final location.
    void step ()
    {
      begin = end;
    }

    /// Extend the current location to the COUNT next columns.
    void columns (counter_type count = 1)
    {
      end += count;
    }

    /// Extend the current location to the COUNT next lines.
    void lines (counter_type count = 1)
    {
      end.lines (count);
    }
    /** \} */


  public:
    /// Beginning of the located region.
    position begin;
    /// End of the located region.
    position end;
  };

  /// Join two locations, in place.
  inline location&
  operator+= (location& res, const location& end)
  {
    res.end = end.end;
    return res;
  }

  /// Join two locations.
  inline location
  operator+ (location res, const location& end)
  {
    return res += end;
  }

  /// Add \a width columns to the end position, in place.
  inline location&
  operator+= (location& res, location::counter_type width)
  {
    res.columns (width);
    return res;
  }

  /// Add \a width columns to the end position.
  inline location
  operator+ (location res, location::counter_type width)
  {
    return res += width;
  }

  /// Subtract \a width columns to the end position, in place.
  inline location&
  operator-= (location& res, location::counter_type width)
  {
    return res += -width;
  }

  /// Subtract \a width columns to the end position.
  inline location
  operator- (location res, location::counter_type width)
  {
    return res -= width;
  }

  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param loc a reference to the location to redirect
   **
   ** Avoid duplicate information.
   */
  template <typename YYChar>
  std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const location& loc)
  {
    location::counter_type end_col
      = 0 < loc.end.column ? loc.end.column - 1 : 0;
    ostr << loc.begin;
    if (loc.end.filename
        && (!loc.begin.filename
            || *loc.begin.filename != *loc.end.filename))
      ostr << '-' << loc.end.filename << ':' << loc.end.line << '.' << end_col;
    else if (loc.begin.line < loc.end.line)
      ostr << '-' << loc.end.line << '.' << end_col;
    else if (loc.begin.column < end_col)
      ostr << '-' << end_col;
    return ostr;
  }


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
      // program
      // def
      // var_list
      // new_var
      // inheritance
      // formal_parameters
      // parameter_list
      // block_or_semicolon
      // block
      // stmt_list
      // stmt
      // const_expr
      // comma_expr
      // expr0
      // lvalue
      // expr4
      // local_decl_list
      // if_stmt
      // opt_else_stmt
      // loop_init
      // opt_comma_expr
      // for_stmt
      // foreach_stmt
      // while_stmt
      // do_stmt
      // switch_stmt
      // try_catch_stmt
      // return_stmt
      // expr_list
      // assoc_list
      // aggregation
      // class_or_name
      // function_call
      // kwarg
      // argument
      // arguments
      // arg_list
      char dummy1[sizeof (ASTNode)];

      // L_TYPE_MODIFIER
      // storage_or_type
      // typed_storage
      // type_modifier_list
      // opt_star
      char dummy2[sizeof (SymbolSignature)];

      // L_REAL_NUMBER
      char dummy3[sizeof (double)];

      // L_TYPE
      // L_ASSIGN
      // L_ORDER
      char dummy4[sizeof (int)];

      // L_INTEGER
      char dummy5[sizeof (long)];

      // function_head
      // new_param
      // id
      char dummy6[sizeof (std::shared_ptr<SymbolTable::Entry>)];

      // L_IDENTIFIER
      // L_STRING_LITERAL
      // opt_as
      // str_literal
      char dummy7[sizeof (std::string)];
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

    /// Symbol locations.
    typedef location location_type;

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const location_type& l, const std::string& m)
        : std::runtime_error (m)
        , location (l)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
        , location (s.location)
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;

      location_type location;
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
    L_AS = 259,                    // L_AS
    L_IF = 260,                    // L_IF
    L_ELSE = 261,                  // L_ELSE
    L_WHILE = 262,                 // L_WHILE
    L_DO = 263,                    // L_DO
    L_FOR = 264,                   // L_FOR
    L_FOREACH = 265,               // L_FOREACH
    L_IN = 266,                    // L_IN
    L_BREAK = 267,                 // L_BREAK
    L_CONTINUE = 268,              // L_CONTINUE
    L_RETURN = 269,                // L_RETURN
    L_SWITCH = 270,                // L_SWITCH
    L_CASE = 271,                  // L_CASE
    L_DEFAULT = 272,               // L_DEFAULT
    L_TRY = 273,                   // L_TRY
    L_CATCH = 274,                 // L_CATCH
    L_NEW = 275,                   // L_NEW
    L_SELF = 276,                  // L_SELF
    LOWER_THAN_ELSE = 277,         // LOWER_THAN_ELSE
    L_TYPE = 278,                  // L_TYPE
    L_ASSIGN = 279,                // L_ASSIGN
    L_ORDER = 280,                 // L_ORDER
    L_INTEGER = 281,               // L_INTEGER
    L_REAL_NUMBER = 282,           // L_REAL_NUMBER
    L_IDENTIFIER = 283,            // L_IDENTIFIER
    L_STRING_LITERAL = 284,        // L_STRING_LITERAL
    L_TYPE_MODIFIER = 285,         // L_TYPE_MODIFIER
    POSITIONAL_IDENTIFIER = 286,   // POSITIONAL_IDENTIFIER
    L_LOR = 287,                   // L_LOR
    L_LAND = 288,                  // L_LAND
    L_LSH = 289,                   // L_LSH
    L_RSH = 290,                   // L_RSH
    L_NOT = 291,                   // L_NOT
    L_INC = 292,                   // L_INC
    L_DEC = 293,                   // L_DEC
    L_ARRAY_OPEN = 294,            // L_ARRAY_OPEN
    L_MAPPING_OPEN = 295,          // L_MAPPING_OPEN
    L_ARROW = 296,                 // L_ARROW
    L_COLON_COLON = 297,           // L_COLON_COLON
    L_ELLIPSIS = 298               // L_ELLIPSIS
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
        YYNTOKENS = 65, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_L_INHERIT = 3,                         // L_INHERIT
        S_L_AS = 4,                              // L_AS
        S_L_IF = 5,                              // L_IF
        S_L_ELSE = 6,                            // L_ELSE
        S_L_WHILE = 7,                           // L_WHILE
        S_L_DO = 8,                              // L_DO
        S_L_FOR = 9,                             // L_FOR
        S_L_FOREACH = 10,                        // L_FOREACH
        S_L_IN = 11,                             // L_IN
        S_L_BREAK = 12,                          // L_BREAK
        S_L_CONTINUE = 13,                       // L_CONTINUE
        S_L_RETURN = 14,                         // L_RETURN
        S_L_SWITCH = 15,                         // L_SWITCH
        S_L_CASE = 16,                           // L_CASE
        S_L_DEFAULT = 17,                        // L_DEFAULT
        S_L_TRY = 18,                            // L_TRY
        S_L_CATCH = 19,                          // L_CATCH
        S_L_NEW = 20,                            // L_NEW
        S_L_SELF = 21,                           // L_SELF
        S_LOWER_THAN_ELSE = 22,                  // LOWER_THAN_ELSE
        S_L_TYPE = 23,                           // L_TYPE
        S_L_ASSIGN = 24,                         // L_ASSIGN
        S_L_ORDER = 25,                          // L_ORDER
        S_L_INTEGER = 26,                        // L_INTEGER
        S_L_REAL_NUMBER = 27,                    // L_REAL_NUMBER
        S_L_IDENTIFIER = 28,                     // L_IDENTIFIER
        S_L_STRING_LITERAL = 29,                 // L_STRING_LITERAL
        S_L_TYPE_MODIFIER = 30,                  // L_TYPE_MODIFIER
        S_POSITIONAL_IDENTIFIER = 31,            // POSITIONAL_IDENTIFIER
        S_32_ = 32,                              // '='
        S_33_ = 33,                              // '?'
        S_L_LOR = 34,                            // L_LOR
        S_L_LAND = 35,                           // L_LAND
        S_36_ = 36,                              // '|'
        S_37_ = 37,                              // '^'
        S_38_ = 38,                              // '&'
        S_39_ = 39,                              // '<'
        S_L_LSH = 40,                            // L_LSH
        S_L_RSH = 41,                            // L_RSH
        S_42_ = 42,                              // '+'
        S_43_ = 43,                              // '-'
        S_44_ = 44,                              // '*'
        S_45_ = 45,                              // '%'
        S_46_ = 46,                              // '/'
        S_L_NOT = 47,                            // L_NOT
        S_48_ = 48,                              // '~'
        S_L_INC = 49,                            // L_INC
        S_L_DEC = 50,                            // L_DEC
        S_L_ARRAY_OPEN = 51,                     // L_ARRAY_OPEN
        S_L_MAPPING_OPEN = 52,                   // L_MAPPING_OPEN
        S_L_ARROW = 53,                          // L_ARROW
        S_L_COLON_COLON = 54,                    // L_COLON_COLON
        S_L_ELLIPSIS = 55,                       // L_ELLIPSIS
        S_56_ = 56,                              // ';'
        S_57_ = 57,                              // '('
        S_58_ = 58,                              // ')'
        S_59_ = 59,                              // ','
        S_60_ = 60,                              // '{'
        S_61_ = 61,                              // '}'
        S_62_ = 62,                              // ':'
        S_63_ = 63,                              // '['
        S_64_ = 64,                              // ']'
        S_YYACCEPT = 65,                         // $accept
        S_all = 66,                              // all
        S_program = 67,                          // program
        S_extra_semicolon = 68,                  // extra_semicolon
        S_def = 69,                              // def
        S_70_1 = 70,                             // $@1
        S_storage_or_type = 71,                  // storage_or_type
        S_typed_storage = 72,                    // typed_storage
        S_type_modifier_list = 73,               // type_modifier_list
        S_function_head = 74,                    // function_head
        S_opt_star = 75,                         // opt_star
        S_var_list = 76,                         // var_list
        S_new_var = 77,                          // new_var
        S_opt_as = 78,                           // opt_as
        S_inheritance = 79,                      // inheritance
        S_str_literal = 80,                      // str_literal
        S_formal_parameters = 81,                // formal_parameters
        S_parameter_list = 82,                   // parameter_list
        S_new_param = 83,                        // new_param
        S_block_or_semicolon = 84,               // block_or_semicolon
        S_block = 85,                            // block
        S_86_2 = 86,                             // $@2
        S_stmt_list = 87,                        // stmt_list
        S_stmt = 88,                             // stmt
        S_label = 89,                            // label
        S_const_expr = 90,                       // const_expr
        S_comma_expr = 91,                       // comma_expr
        S_expr0 = 92,                            // expr0
        S_lvalue = 93,                           // lvalue
        S_expr4 = 94,                            // expr4
        S_local_decl_list = 95,                  // local_decl_list
        S_if_stmt = 96,                          // if_stmt
        S_opt_else_stmt = 97,                    // opt_else_stmt
        S_loop_init = 98,                        // loop_init
        S_opt_comma_expr = 99,                   // opt_comma_expr
        S_for_begin = 100,                       // for_begin
        S_for_stmt = 101,                        // for_stmt
        S_foreach_begin = 102,                   // foreach_begin
        S_foreach_stmt = 103,                    // foreach_stmt
        S_while_begin = 104,                     // while_begin
        S_while_stmt = 105,                      // while_stmt
        S_do_stmt = 106,                         // do_stmt
        S_switch_stmt = 107,                     // switch_stmt
        S_108_3 = 108,                           // $@3
        S_try_catch_stmt = 109,                  // try_catch_stmt
        S_return_stmt = 110,                     // return_stmt
        S_expr_list = 111,                       // expr_list
        S_assoc_list = 112,                      // assoc_list
        S_aggregation = 113,                     // aggregation
        S_id = 114,                              // id
        S_class_or_name = 115,                   // class_or_name
        S_function_call = 116,                   // function_call
        S_kwarg = 117,                           // kwarg
        S_argument = 118,                        // argument
        S_arguments = 119,                       // arguments
        S_arg_list = 120                         // arg_list
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
    /// Provide access to semantic value and location.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol () YY_NOEXCEPT
        : value ()
        , location ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value ()
        , location (std::move (that.location))
      {
        switch (this->kind ())
    {
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.move< ASTNode > (std::move (that.value));
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.move< SymbolSignature > (std::move (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (std::move (that.value));
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (std::move (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.move< long > (std::move (that.value));
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.move< std::shared_ptr<SymbolTable::Entry> > (std::move (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (std::move (that.value));
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
      basic_symbol (typename Base::kind_type t, location_type&& l)
        : Base (t)
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const location_type& l)
        : Base (t)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ASTNode&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ASTNode& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, SymbolSignature&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const SymbolSignature& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, double&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const double& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, int&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const int& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, long&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const long& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::shared_ptr<SymbolTable::Entry>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::shared_ptr<SymbolTable::Entry>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::string&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::string& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
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
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.template destroy< ASTNode > ();
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.template destroy< SymbolSignature > ();
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.template destroy< double > ();
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.template destroy< int > ();
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.template destroy< long > ();
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.template destroy< std::shared_ptr<SymbolTable::Entry> > ();
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.template destroy< std::string > ();
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

      /// The location.
      location_type location;

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
      symbol_type (int tok, location_type l)
        : super_type (token_kind_type (tok), std::move (l))
#else
      symbol_type (int tok, const location_type& l)
        : super_type (token_kind_type (tok), l)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, SymbolSignature v, location_type l)
        : super_type (token_kind_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const SymbolSignature& v, const location_type& l)
        : super_type (token_kind_type (tok), v, l)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, double v, location_type l)
        : super_type (token_kind_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const double& v, const location_type& l)
        : super_type (token_kind_type (tok), v, l)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, int v, location_type l)
        : super_type (token_kind_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const int& v, const location_type& l)
        : super_type (token_kind_type (tok), v, l)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, long v, location_type l)
        : super_type (token_kind_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const long& v, const location_type& l)
        : super_type (token_kind_type (tok), v, l)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, std::string v, location_type l)
        : super_type (token_kind_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const std::string& v, const location_type& l)
        : super_type (token_kind_type (tok), v, l)
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
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

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
      make_YYEOF (location_type l)
      {
        return symbol_type (token::YYEOF, std::move (l));
      }
#else
      static
      symbol_type
      make_YYEOF (const location_type& l)
      {
        return symbol_type (token::YYEOF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYerror (location_type l)
      {
        return symbol_type (token::YYerror, std::move (l));
      }
#else
      static
      symbol_type
      make_YYerror (const location_type& l)
      {
        return symbol_type (token::YYerror, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYUNDEF (location_type l)
      {
        return symbol_type (token::YYUNDEF, std::move (l));
      }
#else
      static
      symbol_type
      make_YYUNDEF (const location_type& l)
      {
        return symbol_type (token::YYUNDEF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_INHERIT (location_type l)
      {
        return symbol_type (token::L_INHERIT, std::move (l));
      }
#else
      static
      symbol_type
      make_L_INHERIT (const location_type& l)
      {
        return symbol_type (token::L_INHERIT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_AS (location_type l)
      {
        return symbol_type (token::L_AS, std::move (l));
      }
#else
      static
      symbol_type
      make_L_AS (const location_type& l)
      {
        return symbol_type (token::L_AS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_IF (location_type l)
      {
        return symbol_type (token::L_IF, std::move (l));
      }
#else
      static
      symbol_type
      make_L_IF (const location_type& l)
      {
        return symbol_type (token::L_IF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ELSE (location_type l)
      {
        return symbol_type (token::L_ELSE, std::move (l));
      }
#else
      static
      symbol_type
      make_L_ELSE (const location_type& l)
      {
        return symbol_type (token::L_ELSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_WHILE (location_type l)
      {
        return symbol_type (token::L_WHILE, std::move (l));
      }
#else
      static
      symbol_type
      make_L_WHILE (const location_type& l)
      {
        return symbol_type (token::L_WHILE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_DO (location_type l)
      {
        return symbol_type (token::L_DO, std::move (l));
      }
#else
      static
      symbol_type
      make_L_DO (const location_type& l)
      {
        return symbol_type (token::L_DO, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_FOR (location_type l)
      {
        return symbol_type (token::L_FOR, std::move (l));
      }
#else
      static
      symbol_type
      make_L_FOR (const location_type& l)
      {
        return symbol_type (token::L_FOR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_FOREACH (location_type l)
      {
        return symbol_type (token::L_FOREACH, std::move (l));
      }
#else
      static
      symbol_type
      make_L_FOREACH (const location_type& l)
      {
        return symbol_type (token::L_FOREACH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_IN (location_type l)
      {
        return symbol_type (token::L_IN, std::move (l));
      }
#else
      static
      symbol_type
      make_L_IN (const location_type& l)
      {
        return symbol_type (token::L_IN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_BREAK (location_type l)
      {
        return symbol_type (token::L_BREAK, std::move (l));
      }
#else
      static
      symbol_type
      make_L_BREAK (const location_type& l)
      {
        return symbol_type (token::L_BREAK, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_CONTINUE (location_type l)
      {
        return symbol_type (token::L_CONTINUE, std::move (l));
      }
#else
      static
      symbol_type
      make_L_CONTINUE (const location_type& l)
      {
        return symbol_type (token::L_CONTINUE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_RETURN (location_type l)
      {
        return symbol_type (token::L_RETURN, std::move (l));
      }
#else
      static
      symbol_type
      make_L_RETURN (const location_type& l)
      {
        return symbol_type (token::L_RETURN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_SWITCH (location_type l)
      {
        return symbol_type (token::L_SWITCH, std::move (l));
      }
#else
      static
      symbol_type
      make_L_SWITCH (const location_type& l)
      {
        return symbol_type (token::L_SWITCH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_CASE (location_type l)
      {
        return symbol_type (token::L_CASE, std::move (l));
      }
#else
      static
      symbol_type
      make_L_CASE (const location_type& l)
      {
        return symbol_type (token::L_CASE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_DEFAULT (location_type l)
      {
        return symbol_type (token::L_DEFAULT, std::move (l));
      }
#else
      static
      symbol_type
      make_L_DEFAULT (const location_type& l)
      {
        return symbol_type (token::L_DEFAULT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_TRY (location_type l)
      {
        return symbol_type (token::L_TRY, std::move (l));
      }
#else
      static
      symbol_type
      make_L_TRY (const location_type& l)
      {
        return symbol_type (token::L_TRY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_CATCH (location_type l)
      {
        return symbol_type (token::L_CATCH, std::move (l));
      }
#else
      static
      symbol_type
      make_L_CATCH (const location_type& l)
      {
        return symbol_type (token::L_CATCH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_NEW (location_type l)
      {
        return symbol_type (token::L_NEW, std::move (l));
      }
#else
      static
      symbol_type
      make_L_NEW (const location_type& l)
      {
        return symbol_type (token::L_NEW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_SELF (location_type l)
      {
        return symbol_type (token::L_SELF, std::move (l));
      }
#else
      static
      symbol_type
      make_L_SELF (const location_type& l)
      {
        return symbol_type (token::L_SELF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LOWER_THAN_ELSE (location_type l)
      {
        return symbol_type (token::LOWER_THAN_ELSE, std::move (l));
      }
#else
      static
      symbol_type
      make_LOWER_THAN_ELSE (const location_type& l)
      {
        return symbol_type (token::LOWER_THAN_ELSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_TYPE (int v, location_type l)
      {
        return symbol_type (token::L_TYPE, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_TYPE (const int& v, const location_type& l)
      {
        return symbol_type (token::L_TYPE, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ASSIGN (int v, location_type l)
      {
        return symbol_type (token::L_ASSIGN, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_ASSIGN (const int& v, const location_type& l)
      {
        return symbol_type (token::L_ASSIGN, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ORDER (int v, location_type l)
      {
        return symbol_type (token::L_ORDER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_ORDER (const int& v, const location_type& l)
      {
        return symbol_type (token::L_ORDER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_INTEGER (long v, location_type l)
      {
        return symbol_type (token::L_INTEGER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_INTEGER (const long& v, const location_type& l)
      {
        return symbol_type (token::L_INTEGER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_REAL_NUMBER (double v, location_type l)
      {
        return symbol_type (token::L_REAL_NUMBER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_REAL_NUMBER (const double& v, const location_type& l)
      {
        return symbol_type (token::L_REAL_NUMBER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_IDENTIFIER (std::string v, location_type l)
      {
        return symbol_type (token::L_IDENTIFIER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_IDENTIFIER (const std::string& v, const location_type& l)
      {
        return symbol_type (token::L_IDENTIFIER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_STRING_LITERAL (std::string v, location_type l)
      {
        return symbol_type (token::L_STRING_LITERAL, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_STRING_LITERAL (const std::string& v, const location_type& l)
      {
        return symbol_type (token::L_STRING_LITERAL, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_TYPE_MODIFIER (SymbolSignature v, location_type l)
      {
        return symbol_type (token::L_TYPE_MODIFIER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_L_TYPE_MODIFIER (const SymbolSignature& v, const location_type& l)
      {
        return symbol_type (token::L_TYPE_MODIFIER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_POSITIONAL_IDENTIFIER (location_type l)
      {
        return symbol_type (token::POSITIONAL_IDENTIFIER, std::move (l));
      }
#else
      static
      symbol_type
      make_POSITIONAL_IDENTIFIER (const location_type& l)
      {
        return symbol_type (token::POSITIONAL_IDENTIFIER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_LOR (location_type l)
      {
        return symbol_type (token::L_LOR, std::move (l));
      }
#else
      static
      symbol_type
      make_L_LOR (const location_type& l)
      {
        return symbol_type (token::L_LOR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_LAND (location_type l)
      {
        return symbol_type (token::L_LAND, std::move (l));
      }
#else
      static
      symbol_type
      make_L_LAND (const location_type& l)
      {
        return symbol_type (token::L_LAND, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_LSH (location_type l)
      {
        return symbol_type (token::L_LSH, std::move (l));
      }
#else
      static
      symbol_type
      make_L_LSH (const location_type& l)
      {
        return symbol_type (token::L_LSH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_RSH (location_type l)
      {
        return symbol_type (token::L_RSH, std::move (l));
      }
#else
      static
      symbol_type
      make_L_RSH (const location_type& l)
      {
        return symbol_type (token::L_RSH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_NOT (location_type l)
      {
        return symbol_type (token::L_NOT, std::move (l));
      }
#else
      static
      symbol_type
      make_L_NOT (const location_type& l)
      {
        return symbol_type (token::L_NOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_INC (location_type l)
      {
        return symbol_type (token::L_INC, std::move (l));
      }
#else
      static
      symbol_type
      make_L_INC (const location_type& l)
      {
        return symbol_type (token::L_INC, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_DEC (location_type l)
      {
        return symbol_type (token::L_DEC, std::move (l));
      }
#else
      static
      symbol_type
      make_L_DEC (const location_type& l)
      {
        return symbol_type (token::L_DEC, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ARRAY_OPEN (location_type l)
      {
        return symbol_type (token::L_ARRAY_OPEN, std::move (l));
      }
#else
      static
      symbol_type
      make_L_ARRAY_OPEN (const location_type& l)
      {
        return symbol_type (token::L_ARRAY_OPEN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_MAPPING_OPEN (location_type l)
      {
        return symbol_type (token::L_MAPPING_OPEN, std::move (l));
      }
#else
      static
      symbol_type
      make_L_MAPPING_OPEN (const location_type& l)
      {
        return symbol_type (token::L_MAPPING_OPEN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ARROW (location_type l)
      {
        return symbol_type (token::L_ARROW, std::move (l));
      }
#else
      static
      symbol_type
      make_L_ARROW (const location_type& l)
      {
        return symbol_type (token::L_ARROW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_COLON_COLON (location_type l)
      {
        return symbol_type (token::L_COLON_COLON, std::move (l));
      }
#else
      static
      symbol_type
      make_L_COLON_COLON (const location_type& l)
      {
        return symbol_type (token::L_COLON_COLON, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_L_ELLIPSIS (location_type l)
      {
        return symbol_type (token::L_ELLIPSIS, std::move (l));
      }
#else
      static
      symbol_type
      make_L_ELLIPSIS (const location_type& l)
      {
        return symbol_type (token::L_ELLIPSIS, l);
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
    typedef short state_type;

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
    static const unsigned char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const short yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const short yydefgoto_[];

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
      yylast_ = 482,     ///< Last index in yytable_.
      yynnts_ = 56,  ///< Number of nonterminal symbols.
      yyfinal_ = 3 ///< Termination state number.
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
       2,     2,     2,     2,     2,     2,     2,    45,    38,     2,
      57,    58,    44,    42,    59,    43,     2,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    62,    56,
      39,    32,     2,    33,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    63,     2,    64,    37,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    60,    36,    61,    48,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    34,    35,    40,
      41,    47,    49,    50,    51,    52,    53,    54,    55
    };
    // Last valid token kind.
    const int code_max = 298;

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
    , location (that.location)
  {
    switch (this->kind ())
    {
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.copy< ASTNode > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.copy< SymbolSignature > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.copy< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.copy< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.copy< long > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.copy< std::shared_ptr<SymbolTable::Entry> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.copy< std::string > (YY_MOVE (that.value));
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
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.move< ASTNode > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.move< SymbolSignature > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.move< long > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.move< std::shared_ptr<SymbolTable::Entry> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
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
#line 2735 "lpc_parser.cpp"


// "%code provides" blocks.
#line 39 "src/lpc_parser.yy"

    // forward declarations for lexer functions (reentrant)
    #undef YY_DECL
    #define YY_DECL yy::LpcParser::symbol_type yylex (yyscan_t yyscanner)
    extern yy::LpcParser::symbol_type yylex (yyscan_t yyscanner);

    // forward declarations for lexer helper functions
    extern YY_EXTRA_TYPE yyget_extra(yyscan_t yyscanner);

    extern int yyget_lineno (yyscan_t yyscanner);
    extern int yyget_column (yyscan_t yyscanner);

    inline ScannerLocation source_location(const yy::location& location, YY_EXTRA_TYPE lpcc) {
        return {
            lpcc && lpcc->consts && location.begin.filename
                ? lpcc->consts->find_or_add(*location.begin.filename)
                : (lpcc ? lpcc->current_file : ConstPool::INVALID_INDEX),
            lpcc ? lpcc->current_func : ConstPool::INVALID_INDEX,
            location.begin.line,
            location.begin.column
        };
    }

    /**
     * Helper macro to assign an AST node (move-assignment assumed) to a target variable.
     * It uses the given arguments and source location to synthesize an AST node.
     * File and function information is retrieved from the compiler context (YY_EXTRA_TYPE).
     * @param target The variable to assign the AST node to.
     * @param location The source location of the AST node as tracked by the Bison parser (yy::location object).
     * @param ... The arguments to construct the AST node.
     */
    #define ASSIGN_AST(target, location, ...) \
        ((target) = ASTNode((__VA_ARGS__), source_location(location, yyget_extra(yyscanner))))

    namespace yy {
        // exception type for parse errors.
        struct parse_error: std::runtime_error {
            parse_error(const std::string& message, int lineno, int column, YY_EXTRA_TYPE lpcc = nullptr)
                : std::runtime_error(message), lineno(lineno), column(column), lpcc(lpcc),
                  filename(lpcc && lpcc->location.begin.filename ? *lpcc->location.begin.filename : "") {}
            parse_error(const std::string& message, const yy::location& location, YY_EXTRA_TYPE lpcc)
                : parse_error(message, location.begin.line, location.begin.column, lpcc) {
                filename = location.begin.filename ? *location.begin.filename : "";
            }
            parse_error(const std::string& message, yyscan_t yyscanner)
                : parse_error(message, yyget_extra(yyscanner)->location.begin.line,
                    yyget_extra(yyscanner)->location.begin.column, yyget_extra(yyscanner)) {}
            int lineno;
            int column;
            YY_EXTRA_TYPE lpcc;
            std::string filename;
        };
    } // namespace yy

#line 2794 "lpc_parser.cpp"






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

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


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
#line 2890 "lpc_parser.cpp"

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
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.YY_MOVE_OR_COPY< ASTNode > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.YY_MOVE_OR_COPY< SymbolSignature > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.YY_MOVE_OR_COPY< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.YY_MOVE_OR_COPY< long > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.YY_MOVE_OR_COPY< std::shared_ptr<SymbolTable::Entry> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
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
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.move< ASTNode > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.move< SymbolSignature > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.move< long > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.move< std::shared_ptr<SymbolTable::Entry> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (YY_MOVE (that.value));
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
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.copy< ASTNode > (that.value);
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.copy< SymbolSignature > (that.value);
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.copy< double > (that.value);
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.copy< int > (that.value);
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.copy< long > (that.value);
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.copy< std::shared_ptr<SymbolTable::Entry> > (that.value);
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  LpcParser::stack_symbol_type&
  LpcParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        value.move< ASTNode > (that.value);
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        value.move< SymbolSignature > (that.value);
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        value.move< double > (that.value);
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        value.move< int > (that.value);
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        value.move< long > (that.value);
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        value.move< std::shared_ptr<SymbolTable::Entry> > (that.value);
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
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
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
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

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

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
      case symbol_kind::S_program: // program
      case symbol_kind::S_def: // def
      case symbol_kind::S_var_list: // var_list
      case symbol_kind::S_new_var: // new_var
      case symbol_kind::S_inheritance: // inheritance
      case symbol_kind::S_formal_parameters: // formal_parameters
      case symbol_kind::S_parameter_list: // parameter_list
      case symbol_kind::S_block_or_semicolon: // block_or_semicolon
      case symbol_kind::S_block: // block
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_stmt: // stmt
      case symbol_kind::S_const_expr: // const_expr
      case symbol_kind::S_comma_expr: // comma_expr
      case symbol_kind::S_expr0: // expr0
      case symbol_kind::S_lvalue: // lvalue
      case symbol_kind::S_expr4: // expr4
      case symbol_kind::S_local_decl_list: // local_decl_list
      case symbol_kind::S_if_stmt: // if_stmt
      case symbol_kind::S_opt_else_stmt: // opt_else_stmt
      case symbol_kind::S_loop_init: // loop_init
      case symbol_kind::S_opt_comma_expr: // opt_comma_expr
      case symbol_kind::S_for_stmt: // for_stmt
      case symbol_kind::S_foreach_stmt: // foreach_stmt
      case symbol_kind::S_while_stmt: // while_stmt
      case symbol_kind::S_do_stmt: // do_stmt
      case symbol_kind::S_switch_stmt: // switch_stmt
      case symbol_kind::S_try_catch_stmt: // try_catch_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_class_or_name: // class_or_name
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_kwarg: // kwarg
      case symbol_kind::S_argument: // argument
      case symbol_kind::S_arguments: // arguments
      case symbol_kind::S_arg_list: // arg_list
        yylhs.value.emplace< ASTNode > ();
        break;

      case symbol_kind::S_L_TYPE_MODIFIER: // L_TYPE_MODIFIER
      case symbol_kind::S_storage_or_type: // storage_or_type
      case symbol_kind::S_typed_storage: // typed_storage
      case symbol_kind::S_type_modifier_list: // type_modifier_list
      case symbol_kind::S_opt_star: // opt_star
        yylhs.value.emplace< SymbolSignature > ();
        break;

      case symbol_kind::S_L_REAL_NUMBER: // L_REAL_NUMBER
        yylhs.value.emplace< double > ();
        break;

      case symbol_kind::S_L_TYPE: // L_TYPE
      case symbol_kind::S_L_ASSIGN: // L_ASSIGN
      case symbol_kind::S_L_ORDER: // L_ORDER
        yylhs.value.emplace< int > ();
        break;

      case symbol_kind::S_L_INTEGER: // L_INTEGER
        yylhs.value.emplace< long > ();
        break;

      case symbol_kind::S_function_head: // function_head
      case symbol_kind::S_new_param: // new_param
      case symbol_kind::S_id: // id
        yylhs.value.emplace< std::shared_ptr<SymbolTable::Entry> > ();
        break;

      case symbol_kind::S_L_IDENTIFIER: // L_IDENTIFIER
      case symbol_kind::S_L_STRING_LITERAL: // L_STRING_LITERAL
      case symbol_kind::S_opt_as: // opt_as
      case symbol_kind::S_str_literal: // str_literal
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // all: program
#line 182 "src/lpc_parser.yy"
            { yyget_extra(yyscanner)->root = std::make_shared<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ())); }
#line 3659 "lpc_parser.cpp"
    break;

  case 3: // program: %empty
#line 186 "src/lpc_parser.yy"
             { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Program{}); }
#line 3665 "lpc_parser.cpp"
    break;

  case 4: // program: program inheritance
#line 188 "src/lpc_parser.yy"
        {
            yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > ();
            if (yystack_[0].value.as < ASTNode > ().node.index() != 0)
                std::get<AST::Program>(yylhs.value.as < ASTNode > ().node).defs.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 3675 "lpc_parser.cpp"
    break;

  case 5: // program: program def extra_semicolon
#line 194 "src/lpc_parser.yy"
        {
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
            if (yystack_[1].value.as < ASTNode > ().node.index() != 0)
                std::get<AST::Program>(yylhs.value.as < ASTNode > ().node).defs.push_back(std::move(yystack_[1].value.as < ASTNode > ()));
        }
#line 3685 "lpc_parser.cpp"
    break;

  case 7: // extra_semicolon: extra_semicolon ';'
#line 203 "src/lpc_parser.yy"
                          { /* ignore extra semicolon(s) between definitions */ }
#line 3691 "lpc_parser.cpp"
    break;

  case 8: // $@1: %empty
#line 208 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup(yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name) && yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->signature.b.is_const)
                throw parse_error("Function already defined: " + yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name, yyscanner);
            lpcc->current_scope->emplace(yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name, yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()); // add the function to the current scope
            lpcc->current_func = lpcc->consts->find_or_add(yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name); // set the current function being parsed
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::FUNCTION, 0); // enter a new subscope for the function body
            lpcc->next_local = 0; // reset overlay base for the function scope
        }
#line 3705 "lpc_parser.cpp"
    break;

  case 9: // def: function_head '(' $@1 formal_parameters ')' block_or_semicolon
#line 218 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            auto func_scope = lpcc->current_scope;
            lpcc->current_scope = func_scope->get_enclosing_scope(); // exit the function's subscope
            lpcc->current_func = ConstPool::INVALID_INDEX; // reset the current function being parsed
            if (yystack_[0].value.as < ASTNode > ().node.index()) {
                yystack_[5].value.as < std::shared_ptr<SymbolTable::Entry> > ()->signature.b.is_const = 1; // mark the function name symbol as const (has body)
                yystack_[5].value.as < std::shared_ptr<SymbolTable::Entry> > ()->value = lpcc->next_func_idx++;
                auto decl = std::make_unique<ASTNode>(AST::FunctionDecl{
                    yystack_[5].value.as < std::shared_ptr<SymbolTable::Entry> > (),
                    std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ()))
                }, source_location(yystack_[5].location, lpcc));
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yystack_[5].location, AST::FunctionDef{
                    std::move(decl),
                    std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
                });
                SPDLOG_DEBUG("Added definition of function: {}", yystack_[5].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name);
            }
            else {
                // no function body => a forward declaration (allows calling a function before its definition)
                auto lpcc = yyget_extra(yyscanner);
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yystack_[5].location, AST::FunctionDecl{
                    yystack_[5].value.as < std::shared_ptr<SymbolTable::Entry> > (),
                    std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ()))
                });
                SPDLOG_DEBUG("Forward declaration of function: {}", yystack_[5].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name);
            }
        }
#line 3738 "lpc_parser.cpp"
    break;

  case 10: // def: storage_or_type var_list ';'
#line 247 "src/lpc_parser.yy"
        {
            /* variable declarations */
            yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > ();
        }
#line 3747 "lpc_parser.cpp"
    break;

  case 11: // storage_or_type: type_modifier_list
#line 255 "src/lpc_parser.yy"
        {
            (yylhs.value.as < SymbolSignature > () = yystack_[0].value.as < SymbolSignature > ()).b.data_type = LpcDataType::T_INT;
            yyget_extra(yyscanner)->variable_signature = yylhs.value.as < SymbolSignature > ();
        }
#line 3756 "lpc_parser.cpp"
    break;

  case 12: // storage_or_type: typed_storage
#line 260 "src/lpc_parser.yy"
        {
            yylhs.value.as < SymbolSignature > () = yystack_[0].value.as < SymbolSignature > ();
            yyget_extra(yyscanner)->variable_signature = yylhs.value.as < SymbolSignature > ();
        }
#line 3765 "lpc_parser.cpp"
    break;

  case 13: // typed_storage: L_TYPE
#line 268 "src/lpc_parser.yy"
        { yylhs.value.as < SymbolSignature > () = SymbolSignature{}; yylhs.value.as < SymbolSignature > ().b.data_type = yystack_[0].value.as < int > (); }
#line 3771 "lpc_parser.cpp"
    break;

  case 14: // typed_storage: type_modifier_list L_TYPE
#line 270 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = yystack_[1].value.as < SymbolSignature > ()).b.data_type = yystack_[0].value.as < int > (); }
#line 3777 "lpc_parser.cpp"
    break;

  case 15: // type_modifier_list: L_TYPE_MODIFIER
#line 274 "src/lpc_parser.yy"
      { yylhs.value.as < SymbolSignature > () = yystack_[0].value.as < SymbolSignature > (); }
#line 3783 "lpc_parser.cpp"
    break;

  case 16: // type_modifier_list: type_modifier_list L_TYPE_MODIFIER
#line 276 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = yystack_[1].value.as < SymbolSignature > ()).value |= yystack_[0].value.as < SymbolSignature > ().value; }
#line 3789 "lpc_parser.cpp"
    break;

  case 17: // function_head: opt_star L_IDENTIFIER
#line 281 "src/lpc_parser.yy"
        {
            SymbolSignature sig{0};
            sig.b.data_type = LpcDataType::T_INT; // implicit function return type
            sig.b.is_function = 1; // mark as function
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), sig);
            if (!yystack_[1].value.as < SymbolSignature > ().b.is_array)
                yylhs.location.begin = yystack_[0].location.begin;
        }
#line 3802 "lpc_parser.cpp"
    break;

  case 18: // function_head: storage_or_type opt_star L_IDENTIFIER
#line 290 "src/lpc_parser.yy"
        {
            yystack_[2].value.as < SymbolSignature > ().value |= yystack_[1].value.as < SymbolSignature > ().value;
            yystack_[2].value.as < SymbolSignature > ().b.is_function = 1; // mark as function
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), yystack_[2].value.as < SymbolSignature > ());
        }
#line 3812 "lpc_parser.cpp"
    break;

  case 19: // opt_star: %empty
#line 299 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = SymbolSignature{}).value = 0; }
#line 3818 "lpc_parser.cpp"
    break;

  case 20: // opt_star: '*'
#line 301 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = SymbolSignature{}).b.is_array = 1; /* mark as array */ }
#line 3824 "lpc_parser.cpp"
    break;

  case 21: // var_list: new_var
#line 305 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 3830 "lpc_parser.cpp"
    break;

  case 22: // var_list: var_list ',' new_var
#line 307 "src/lpc_parser.yy"
        {
            if (yystack_[0].value.as < ASTNode > ().node.index() == 0) {
                yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
            }
            else if (yystack_[2].value.as < ASTNode > ().node.index() == 0) {
                yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > ();
            }
            else {
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yystack_[2].location, AST::BinaryOp{
                    oparg::COMMA,
                    std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                    std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
                });
            }
        }
#line 3850 "lpc_parser.cpp"
    break;

  case 23: // new_var: opt_star L_IDENTIFIER
#line 326 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup(yystack_[0].value.as < std::string > ()))
                throw parse_error("Variable already defined: " + yystack_[0].value.as < std::string > (), yyscanner);
            auto signature = lpcc->variable_signature;
            signature.value |= yystack_[1].value.as < SymbolSignature > ().value;
            auto entry = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), signature);
            if (lpcc->current_scope->get_enclosing_scope()) { // LOCAL or BLOCK scope
                entry->value = uint32_t{lpcc->next_local++};
                entry->signature.b.is_local = 1; // mark as local variable
            }
            else
                entry->value = uint32_t{lpcc->next_global++};
            lpcc->current_scope->emplace(yystack_[0].value.as < std::string > (), entry); // add the variable to current scope
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yystack_[0].location, std::monostate{}); // no-op
            if (!yystack_[1].value.as < SymbolSignature > ().b.is_array)
                yylhs.location.begin = yystack_[0].location.begin;
        }
#line 3873 "lpc_parser.cpp"
    break;

  case 24: // new_var: opt_star L_IDENTIFIER '=' expr0
#line 345 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup(yystack_[2].value.as < std::string > ()))
                throw parse_error("Variable already defined: " + yystack_[2].value.as < std::string > (), yyscanner);
            auto signature = lpcc->variable_signature;
            signature.value |= yystack_[3].value.as < SymbolSignature > ().value;
            auto entry = std::make_shared<SymbolTable::Entry>(yystack_[2].value.as < std::string > (), signature);
            if (lpcc->current_scope->get_enclosing_scope()) { // LOCAL or BLOCK scope
                entry->value = uint32_t{lpcc->next_local++};
                entry->signature.b.is_local = 1; // mark as local variable
            }
            else
                entry->value = uint32_t{lpcc->next_global++};
            lpcc->current_scope->emplace(yystack_[2].value.as < std::string > (), entry); // add the variable to current scope
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yystack_[2].location, AST::Assign{
                oparg::ASSIGN,
                std::make_unique<ASTNode>(AST::Variable{ std::move(entry) }, source_location(yystack_[2].location, lpcc)), // lhs
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
            if (!yystack_[3].value.as < SymbolSignature > ().b.is_array)
                yylhs.location.begin = yystack_[2].location.begin;
        }
#line 3900 "lpc_parser.cpp"
    break;

  case 25: // opt_as: %empty
#line 371 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = ""; }
#line 3906 "lpc_parser.cpp"
    break;

  case 26: // opt_as: L_AS L_IDENTIFIER
#line 373 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = yystack_[0].value.as < std::string > (); }
#line 3912 "lpc_parser.cpp"
    break;

  case 27: // inheritance: L_INHERIT str_literal opt_as ';'
#line 378 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            std::string alias{yystack_[1].value.as < std::string > ().empty() ? yystack_[2].value.as < std::string > () : yystack_[1].value.as < std::string > ()};
            if (!lpcc->current_scope->lookup(alias)) {
                auto entry = std::make_shared<SymbolTable::Entry>(alias, SymbolSignature{0}); // alias symbol
                entry->value = lpcc->current_scope->new_token(yystack_[2].value.as < std::string > (), SymbolTable::TokenType::STRING_LITERAL);
                lpcc->current_scope->emplace(alias, entry); // add the pseudo entry to current scope
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Inheritance{ entry });
            }
            else if (!yystack_[1].value.as < std::string > ().empty()) {
                throw parse_error("Alias already defined: " + alias, yyscanner);
            }
        }
#line 3930 "lpc_parser.cpp"
    break;

  case 28: // inheritance: type_modifier_list L_INHERIT str_literal opt_as ';'
#line 392 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            std::string alias{yystack_[1].value.as < std::string > ().empty() ? yystack_[2].value.as < std::string > () : yystack_[1].value.as < std::string > ()};
            if (!lpcc->current_scope->lookup(alias)) {
                auto entry = std::make_shared<SymbolTable::Entry>(alias, yystack_[4].value.as < SymbolSignature > ()); // alias symbol
                entry->value = lpcc->current_scope->new_token(yystack_[2].value.as < std::string > (), SymbolTable::TokenType::STRING_LITERAL);
                lpcc->current_scope->emplace(alias, entry); // add the pseudo entry to current scope
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Inheritance{ entry });
            }
            else if (!yystack_[1].value.as < std::string > ().empty()) {
                throw parse_error("Alias already defined: " + alias, yyscanner);
            }
        }
#line 3948 "lpc_parser.cpp"
    break;

  case 29: // str_literal: L_STRING_LITERAL
#line 409 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = yystack_[0].value.as < std::string > (); }
#line 3954 "lpc_parser.cpp"
    break;

  case 30: // str_literal: str_literal L_STRING_LITERAL
#line 411 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = yystack_[1].value.as < std::string > () + yystack_[0].value.as < std::string > (); /* compile-time string concatenation */ }
#line 3960 "lpc_parser.cpp"
    break;

  case 31: // formal_parameters: %empty
#line 416 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ParameterList{}); }
#line 3966 "lpc_parser.cpp"
    break;

  case 32: // formal_parameters: parameter_list
#line 417 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 3972 "lpc_parser.cpp"
    break;

  case 33: // parameter_list: new_param
#line 422 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ParameterList{ std::vector<std::shared_ptr<SymbolTable::Entry>>{std::move(yystack_[0].value.as < std::shared_ptr<SymbolTable::Entry> > ())} }); }
#line 3978 "lpc_parser.cpp"
    break;

  case 34: // parameter_list: parameter_list ',' new_param
#line 424 "src/lpc_parser.yy"
        {
            auto& params = std::get<AST::ParameterList>(yystack_[2].value.as < ASTNode > ().node).parameters;
            if(params.back()->signature.b.data_type == LpcDataType::T_VOID)
                throw parse_error("Cannot have parameters after a void parameter", yyscanner);
            if(params.back()->signature.b.data_type == LpcDataType::T_VARARG)
                throw parse_error("Cannot have parameters after a vararg parameter", yyscanner);
            params.push_back(std::move(yystack_[0].value.as < std::shared_ptr<SymbolTable::Entry> > ()));
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
        }
#line 3992 "lpc_parser.cpp"
    break;

  case 35: // new_param: L_TYPE opt_star L_IDENTIFIER
#line 437 "src/lpc_parser.yy"
        {
            SymbolSignature s = yystack_[1].value.as < SymbolSignature > ();
            s.b.data_type = yystack_[2].value.as < int > ();
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup(yystack_[0].value.as < std::string > ()))
                throw parse_error("Parameter name already defined: " + std::string(yystack_[0].value.as < std::string > ()), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace(yystack_[0].value.as < std::string > (), entry);
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = entry;
        }
#line 4009 "lpc_parser.cpp"
    break;

  case 36: // new_param: L_TYPE opt_star
#line 450 "src/lpc_parser.yy"
        {
            SymbolSignature s = yystack_[0].value.as < SymbolSignature > ();
            s.b.data_type = yystack_[1].value.as < int > ();
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local++; // increment local variable counter for anonymous parameter
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = std::make_shared<SymbolTable::Entry>("", s);
        }
#line 4021 "lpc_parser.cpp"
    break;

  case 37: // new_param: L_IDENTIFIER
#line 458 "src/lpc_parser.yy"
        {
            SymbolSignature s{LpcDataType::T_INT}; // default type for untyped parameter
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup(yystack_[0].value.as < std::string > ()))
                throw parse_error("Parameter name already defined: " + std::string(yystack_[0].value.as < std::string > ()), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace(yystack_[0].value.as < std::string > (), entry);
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = entry;
        }
#line 4037 "lpc_parser.cpp"
    break;

  case 38: // new_param: L_ELLIPSIS L_IDENTIFIER
#line 470 "src/lpc_parser.yy"
        {
            SymbolSignature s{LpcDataType::T_VARARG}; // vararg parameter, a catch-all array for remaining actual parameters
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup(yystack_[0].value.as < std::string > ()))
                throw parse_error("Parameter name already defined: " + std::string(yystack_[0].value.as < std::string > ()), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace(yystack_[0].value.as < std::string > (), entry);
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = entry;
        }
#line 4053 "lpc_parser.cpp"
    break;

  case 39: // block_or_semicolon: ';'
#line 485 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); }
#line 4059 "lpc_parser.cpp"
    break;

  case 40: // block_or_semicolon: block
#line 486 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4065 "lpc_parser.cpp"
    break;

  case 41: // $@2: %empty
#line 491 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::FUNCTION) {
                // The lpcc->current_scope contains formal parameters of the function.
                // Now entering the local scope for the function body.
                lpcc->current_scope->set_scope_type(SymbolTable::ScopeType::LOCAL);
            }
            else if (lpcc->current_scope->scope_type() != SymbolTable::ScopeType::GLOBAL) {
                lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::BLOCK, lpcc->next_local);
            }
        }
#line 4081 "lpc_parser.cpp"
    break;

  case 42: // block: '{' $@2 stmt_list '}'
#line 503 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            auto num_locals = lpcc->next_local;
            if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::BLOCK) {
                num_locals -= lpcc->current_scope->overlay_base();
                lpcc->next_local = lpcc->current_scope->overlay_base(); // restore the local variable counter to the overlay base of the enclosing block
                lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            }
            else if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::LOCAL) {
                lpcc->next_local = 0;
            }
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Block{
                .num_locals = num_locals,
                .stmts = std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))
            });
        }
#line 4102 "lpc_parser.cpp"
    break;

  case 43: // stmt_list: %empty
#line 523 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{}); /* No-op */ }
#line 4108 "lpc_parser.cpp"
    break;

  case 44: // stmt_list: stmt_list stmt
#line 525 "src/lpc_parser.yy"
        {
            if (yystack_[0].value.as < ASTNode > ().node.index() != 0) {
                if (std::get<AST::List>(yystack_[1].value.as < ASTNode > ().node).elts.empty())
                    yystack_[1].value.as < ASTNode > ().src = yystack_[0].value.as < ASTNode > ().src;
                std::get<AST::List>(yystack_[1].value.as < ASTNode > ().node).elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            }
            yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > ();
        }
#line 4121 "lpc_parser.cpp"
    break;

  case 45: // stmt: label stmt
#line 537 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4127 "lpc_parser.cpp"
    break;

  case 46: // stmt: comma_expr ';'
#line 538 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4133 "lpc_parser.cpp"
    break;

  case 47: // stmt: return_stmt
#line 539 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4139 "lpc_parser.cpp"
    break;

  case 48: // stmt: local_decl_list
#line 540 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4145 "lpc_parser.cpp"
    break;

  case 49: // stmt: do_stmt
#line 541 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4151 "lpc_parser.cpp"
    break;

  case 50: // stmt: for_stmt
#line 542 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4157 "lpc_parser.cpp"
    break;

  case 51: // stmt: foreach_stmt
#line 543 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4163 "lpc_parser.cpp"
    break;

  case 52: // stmt: if_stmt
#line 544 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4169 "lpc_parser.cpp"
    break;

  case 53: // stmt: while_stmt
#line 545 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4175 "lpc_parser.cpp"
    break;

  case 54: // stmt: switch_stmt
#line 546 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4181 "lpc_parser.cpp"
    break;

  case 55: // stmt: try_catch_stmt
#line 547 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4187 "lpc_parser.cpp"
    break;

  case 56: // stmt: block
#line 548 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4193 "lpc_parser.cpp"
    break;

  case 57: // stmt: L_BREAK ';'
#line 550 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Break{}); }
#line 4199 "lpc_parser.cpp"
    break;

  case 58: // stmt: L_CONTINUE ';'
#line 552 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Continue{}); }
#line 4205 "lpc_parser.cpp"
    break;

  case 59: // label: L_CASE const_expr ':'
#line 558 "src/lpc_parser.yy"
        {
            if (yyget_extra(yyscanner)->switch_depth == 0)
                throw parse_error("case label outside switch", yystack_[2].location, yyget_extra(yyscanner));
        }
#line 4214 "lpc_parser.cpp"
    break;

  case 60: // label: L_DEFAULT ':'
#line 563 "src/lpc_parser.yy"
        {
            if (yyget_extra(yyscanner)->switch_depth == 0)
                throw parse_error("default label outside switch", yystack_[1].location, yyget_extra(yyscanner));
        }
#line 4223 "lpc_parser.cpp"
    break;

  case 61: // const_expr: L_INTEGER
#line 571 "src/lpc_parser.yy"
        {
            auto consts = yyget_extra(yyscanner)->consts;
            if (yystack_[0].value.as < long > () > 0) {
                // determine if $1 fits in an unsigned 32-bit integer
                if (yystack_[0].value.as < long > () > std::numeric_limits<uint32_t>::min() && yystack_[0].value.as < long > () < std::numeric_limits<uint32_t>::max()) {
                    uint32_t u32 = static_cast<uint32_t>(yystack_[0].value.as < long > ());
                    ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::NumericConst{ u32, consts->find_or_add(u32) });
                }
                else
                    ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::NumericConst{ yystack_[0].value.as < long > (), consts->find_or_add(yystack_[0].value.as < long > ()) });
            }
            else {
                // determine if $1 fits in a signed 32-bit integer
                if (yystack_[0].value.as < long > () > std::numeric_limits<int32_t>::min() && yystack_[0].value.as < long > () < std::numeric_limits<int32_t>::max()) {
                    int32_t i32 = static_cast<int32_t>(yystack_[0].value.as < long > ());
                    ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::NumericConst{ i32, consts->find_or_add(i32) });
                }
                else
                        ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::NumericConst{ yystack_[0].value.as < long > (), consts->find_or_add(yystack_[0].value.as < long > ()) });
            }
        }
#line 4249 "lpc_parser.cpp"
    break;

  case 62: // const_expr: L_REAL_NUMBER
#line 593 "src/lpc_parser.yy"
        {
            auto consts = yyget_extra(yyscanner)->consts;
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::NumericConst{ yystack_[0].value.as < double > (), consts->find_or_add(yystack_[0].value.as < double > ()) });
        }
#line 4258 "lpc_parser.cpp"
    break;

  case 63: // const_expr: str_literal
#line 598 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::StringLiteral{yystack_[0].value.as < std::string > (), *yyget_extra(yyscanner)->consts}); }
#line 4264 "lpc_parser.cpp"
    break;

  case 64: // comma_expr: expr0
#line 602 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4270 "lpc_parser.cpp"
    break;

  case 65: // comma_expr: comma_expr ',' expr0
#line 604 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::COMMA, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4276 "lpc_parser.cpp"
    break;

  case 66: // expr0: lvalue L_ASSIGN expr0
#line 609 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{yystack_[1].value.as < int > (), std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4282 "lpc_parser.cpp"
    break;

  case 67: // expr0: lvalue '=' expr0
#line 611 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4288 "lpc_parser.cpp"
    break;

  case 68: // expr0: L_INC lvalue
#line 613 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_PREFIX_INC, nullptr, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4294 "lpc_parser.cpp"
    break;

  case 69: // expr0: L_DEC lvalue
#line 615 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_PREFIX_DEC, nullptr, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4300 "lpc_parser.cpp"
    break;

  case 70: // expr0: lvalue L_INC
#line 617 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_SUFFIX_INC, std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())), nullptr}); }
#line 4306 "lpc_parser.cpp"
    break;

  case 71: // expr0: lvalue L_DEC
#line 619 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_SUFFIX_DEC, std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())), nullptr}); }
#line 4312 "lpc_parser.cpp"
    break;

  case 72: // expr0: expr0 '?' expr0 ':' expr0
#line 621 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::TernaryOp{
                std::make_unique<ASTNode>(std::move(yystack_[4].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}
            );
        }
#line 4324 "lpc_parser.cpp"
    break;

  case 73: // expr0: expr0 L_LOR expr0
#line 629 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::LOGICAL_OR, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4330 "lpc_parser.cpp"
    break;

  case 74: // expr0: expr0 L_LAND expr0
#line 631 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::LOGICAL_AND, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4336 "lpc_parser.cpp"
    break;

  case 75: // expr0: expr0 '|' expr0
#line 633 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::BITWISE_OR, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4342 "lpc_parser.cpp"
    break;

  case 76: // expr0: expr0 '^' expr0
#line 635 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::BITWISE_XOR, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4348 "lpc_parser.cpp"
    break;

  case 77: // expr0: expr0 '&' expr0
#line 637 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::BITWISE_AND, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4354 "lpc_parser.cpp"
    break;

  case 78: // expr0: expr0 L_LSH expr0
#line 639 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::LSH, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4360 "lpc_parser.cpp"
    break;

  case 79: // expr0: expr0 L_RSH expr0
#line 641 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::RSH, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4366 "lpc_parser.cpp"
    break;

  case 80: // expr0: expr0 '+' expr0
#line 643 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::ADD, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4372 "lpc_parser.cpp"
    break;

  case 81: // expr0: expr0 '-' expr0
#line 645 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::SUB, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4378 "lpc_parser.cpp"
    break;

  case 82: // expr0: expr0 '*' expr0
#line 647 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::MUL, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4384 "lpc_parser.cpp"
    break;

  case 83: // expr0: expr0 '/' expr0
#line 649 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::DIV, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4390 "lpc_parser.cpp"
    break;

  case 84: // expr0: expr0 '%' expr0
#line 651 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::MOD, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4396 "lpc_parser.cpp"
    break;

  case 85: // expr0: expr0 L_ORDER expr0
#line 653 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{yystack_[1].value.as < int > (), std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4402 "lpc_parser.cpp"
    break;

  case 86: // expr0: L_NOT expr0
#line 655 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::UnaryOp{oparg::NOT, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4408 "lpc_parser.cpp"
    break;

  case 87: // expr0: '~' expr0
#line 657 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::UnaryOp{oparg::BITWISE_NOT, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4414 "lpc_parser.cpp"
    break;

  case 88: // expr0: '-' expr0
#line 659 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::UnaryOp{oparg::NEG, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4420 "lpc_parser.cpp"
    break;

  case 89: // expr0: expr4
#line 660 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4426 "lpc_parser.cpp"
    break;

  case 90: // expr0: const_expr
#line 661 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4432 "lpc_parser.cpp"
    break;

  case 91: // lvalue: expr4
#line 666 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4438 "lpc_parser.cpp"
    break;

  case 92: // expr4: '(' comma_expr ')'
#line 671 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4444 "lpc_parser.cpp"
    break;

  case 93: // expr4: L_SELF
#line 673 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Self{ }); }
#line 4450 "lpc_parser.cpp"
    break;

  case 94: // expr4: L_IDENTIFIER
#line 675 "src/lpc_parser.yy"
        {
            auto entry = yyget_extra(yyscanner)->current_scope->lookup(std::string(yystack_[0].value.as < std::string > ()));
            if (!entry) {
                // Handle the error, e.g., throw an exception or report an undefined variable
                throw parse_error("Undefined variable: " + std::string(yystack_[0].value.as < std::string > ()), yyscanner);
            }
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Variable{ entry });
        }
#line 4463 "lpc_parser.cpp"
    break;

  case 95: // expr4: function_call
#line 683 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4469 "lpc_parser.cpp"
    break;

  case 96: // expr4: aggregation
#line 684 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4475 "lpc_parser.cpp"
    break;

  case 97: // expr4: expr4 '[' comma_expr ']'
#line 685 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[3].value.as < ASTNode > (); }
#line 4481 "lpc_parser.cpp"
    break;

  case 98: // local_decl_list: ';'
#line 690 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); /* i.e. No-op */ }
#line 4487 "lpc_parser.cpp"
    break;

  case 99: // local_decl_list: storage_or_type var_list ';'
#line 692 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4493 "lpc_parser.cpp"
    break;

  case 100: // if_stmt: L_IF '(' comma_expr ')' stmt opt_else_stmt
#line 697 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Cond{
                std::make_unique<ASTNode>(std::move(yystack_[3].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4505 "lpc_parser.cpp"
    break;

  case 101: // opt_else_stmt: %empty
#line 708 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); }
#line 4511 "lpc_parser.cpp"
    break;

  case 102: // opt_else_stmt: L_ELSE stmt
#line 710 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > ();}
#line 4517 "lpc_parser.cpp"
    break;

  case 103: // loop_init: local_decl_list
#line 714 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4523 "lpc_parser.cpp"
    break;

  case 104: // loop_init: comma_expr ';'
#line 715 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4529 "lpc_parser.cpp"
    break;

  case 105: // opt_comma_expr: %empty
#line 720 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); }
#line 4535 "lpc_parser.cpp"
    break;

  case 106: // opt_comma_expr: comma_expr
#line 721 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4541 "lpc_parser.cpp"
    break;

  case 107: // for_begin: L_FOR '('
#line 726 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local);
        }
#line 4550 "lpc_parser.cpp"
    break;

  case 108: // for_stmt: for_begin loop_init opt_comma_expr ';' opt_comma_expr ')' stmt
#line 734 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            // leave the loop scope, releasing the locals declared in the init clause
            const uint32_t num_locals = lpcc->next_local - lpcc->current_scope->overlay_base();
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            ASTNode for_loop;
            ASSIGN_AST(for_loop, yylhs.location, AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move(yystack_[4].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
            if (yystack_[5].value.as < ASTNode > ().node.index() != 0) {
                std::vector<ASTNode> stmts;
                auto list_source = yystack_[5].value.as < ASTNode > ().src;
                stmts.push_back(std::move(yystack_[5].value.as < ASTNode > ()));
                stmts.push_back(std::move(for_loop));
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Block{ num_locals, std::make_unique<ASTNode>(AST::List{ std::move(stmts) }, list_source) });
            }
            else {
                yylhs.value.as < ASTNode > () = for_loop;
            }
        }
#line 4578 "lpc_parser.cpp"
    break;

  case 109: // foreach_begin: L_FOREACH '('
#line 761 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local);
        }
#line 4587 "lpc_parser.cpp"
    break;

  case 110: // foreach_stmt: foreach_begin comma_expr L_IN expr4 ')' stmt
#line 769 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::IterativeLoop{
                std::make_unique<ASTNode>(std::move(yystack_[4].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4602 "lpc_parser.cpp"
    break;

  case 111: // while_begin: L_WHILE '('
#line 783 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local);
        }
#line 4611 "lpc_parser.cpp"
    break;

  case 112: // while_stmt: while_begin comma_expr ')' stmt
#line 791 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                nullptr,
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4626 "lpc_parser.cpp"
    break;

  case 113: // while_stmt: while_begin loop_init comma_expr ')' stmt
#line 802 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            // leave the loop scope, releasing the locals declared in the init clause
            const uint32_t num_locals = lpcc->next_local - lpcc->current_scope->overlay_base();
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            ASTNode loop;
            ASSIGN_AST(loop, yylhs.location, AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                nullptr,
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
            if (yystack_[3].value.as < ASTNode > ().node.index() != 0) {
                std::vector<ASTNode> stmts;
                auto list_source = yystack_[3].value.as < ASTNode > ().src;
                stmts.push_back(std::move(yystack_[3].value.as < ASTNode > ()));
                stmts.push_back(std::move(loop));
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Block{ num_locals, std::make_unique<ASTNode>(AST::List{ std::move(stmts) }, list_source) });
            }
            else {
                yylhs.value.as < ASTNode > () = loop;
            }
        }
#line 4654 "lpc_parser.cpp"
    break;

  case 114: // do_stmt: L_DO stmt L_WHILE '(' comma_expr ')' ';'
#line 829 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::PostTestedLoop{
                std::make_unique<ASTNode>(std::move(yystack_[5].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ()))
            });
        }
#line 4665 "lpc_parser.cpp"
    break;

  case 115: // $@3: %empty
#line 839 "src/lpc_parser.yy"
        { ++yyget_extra(yyscanner)->switch_depth; }
#line 4671 "lpc_parser.cpp"
    break;

  case 116: // switch_stmt: L_SWITCH '(' comma_expr ')' $@3 stmt
#line 841 "src/lpc_parser.yy"
        {
            --yyget_extra(yyscanner)->switch_depth;
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Switch{
                std::make_unique<ASTNode>(std::move(yystack_[3].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4683 "lpc_parser.cpp"
    break;

  case 117: // try_catch_stmt: L_TRY stmt L_CATCH '(' parameter_list ')' stmt
#line 852 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::TryCatch{
                std::make_unique<ASTNode>(std::move(yystack_[5].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4695 "lpc_parser.cpp"
    break;

  case 118: // return_stmt: L_RETURN ';'
#line 863 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Return{nullptr}); }
#line 4701 "lpc_parser.cpp"
    break;

  case 119: // return_stmt: L_RETURN comma_expr ';'
#line 865 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Return{std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))}); }
#line 4707 "lpc_parser.cpp"
    break;

  case 120: // expr_list: expr0
#line 870 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{});
            std::get<AST::List>(yylhs.value.as < ASTNode > ().node).elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 4716 "lpc_parser.cpp"
    break;

  case 121: // expr_list: expr_list ',' expr0
#line 875 "src/lpc_parser.yy"
        {
            auto &elts = std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts;
            elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
        }
#line 4726 "lpc_parser.cpp"
    break;

  case 122: // assoc_list: expr0 ':' expr0
#line 884 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{});
            auto& elts = std::get<AST::List>(yylhs.value.as < ASTNode > ().node).elts;
            elts.push_back(std::move(yystack_[2].value.as < ASTNode > ()));
            elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 4737 "lpc_parser.cpp"
    break;

  case 123: // assoc_list: assoc_list ',' expr0 ':' expr0
#line 891 "src/lpc_parser.yy"
        {
            auto &elts = std::get<AST::List>(yystack_[4].value.as < ASTNode > ().node).elts;
            elts.push_back(std::move(yystack_[2].value.as < ASTNode > ()));
            elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            yylhs.value.as < ASTNode > () = yystack_[4].value.as < ASTNode > ();
        }
#line 4748 "lpc_parser.cpp"
    break;

  case 124: // aggregation: L_ARRAY_OPEN expr_list '}' ')'
#line 901 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{ std::move(std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts) }); }
#line 4754 "lpc_parser.cpp"
    break;

  case 125: // aggregation: L_MAPPING_OPEN assoc_list ']' ')'
#line 903 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{ std::move(std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts) }); }
#line 4760 "lpc_parser.cpp"
    break;

  case 126: // id: L_IDENTIFIER
#line 908 "src/lpc_parser.yy"
        { yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = yyget_extra(yyscanner)->current_scope->lookup(yystack_[0].value.as < std::string > ()); }
#line 4766 "lpc_parser.cpp"
    break;

  case 127: // id: id L_COLON_COLON L_IDENTIFIER
#line 909 "src/lpc_parser.yy"
      { yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = yystack_[2].value.as < std::shared_ptr<SymbolTable::Entry> > (); }
#line 4772 "lpc_parser.cpp"
    break;

  case 128: // class_or_name: id
#line 914 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Variable{ std::move(yystack_[0].value.as < std::shared_ptr<SymbolTable::Entry> > ()) }); }
#line 4778 "lpc_parser.cpp"
    break;

  case 129: // class_or_name: str_literal
#line 916 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::StringLiteral{ yystack_[0].value.as < std::string > (), *yyget_extra(yyscanner)->consts }); }
#line 4784 "lpc_parser.cpp"
    break;

  case 130: // function_call: id '(' arg_list ')'
#line 921 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            if (!yystack_[3].value.as < std::shared_ptr<SymbolTable::Entry> > ())
                throw std::runtime_error("Function not found");
            yystack_[3].value.as < std::shared_ptr<SymbolTable::Entry> > ()->signature.b.referenced = 1; // mark the function as referenced
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::FunctionCall{ std::move(yystack_[3].value.as < std::shared_ptr<SymbolTable::Entry> > ()), std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())) });
        }
#line 4796 "lpc_parser.cpp"
    break;

  case 131: // function_call: L_NEW class_or_name '{' arg_list '}'
#line 929 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::New{
                std::make_unique<ASTNode>(std::move(yystack_[3].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))
            });
        }
#line 4807 "lpc_parser.cpp"
    break;

  case 132: // function_call: expr4 L_ARROW L_IDENTIFIER '(' arg_list ')'
#line 936 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::CallOther{
                std::make_unique<ASTNode>(std::move(yystack_[5].value.as < ASTNode > ())),
                yyget_extra(yyscanner)->consts->find_or_add(yystack_[3].value.as < std::string > ()),
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))
            });
        }
#line 4819 "lpc_parser.cpp"
    break;

  case 133: // kwarg: L_IDENTIFIER '=' expr0
#line 947 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::KeywordArgument{ yystack_[2].value.as < std::string > (), *yyget_extra(yyscanner)->consts });
            std::get<AST::KeywordArgument>(yylhs.value.as < ASTNode > ().node).value = std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 4828 "lpc_parser.cpp"
    break;

  case 134: // argument: expr0
#line 954 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4834 "lpc_parser.cpp"
    break;

  case 135: // argument: kwarg
#line 955 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4840 "lpc_parser.cpp"
    break;

  case 136: // arguments: argument
#line 960 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{});
            std::get<AST::List>(yylhs.value.as < ASTNode > ().node).elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 4849 "lpc_parser.cpp"
    break;

  case 137: // arguments: arguments ',' argument
#line 965 "src/lpc_parser.yy"
        {
            auto& args = std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts;
            if (std::holds_alternative<AST::KeywordArgument>(args.back().node)
                && !std::holds_alternative<AST::KeywordArgument>(yystack_[0].value.as < ASTNode > ().node))
                throw parse_error("Positional argument after keyword argument", yystack_[0].location, yyget_extra(yyscanner));
            args.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
        }
#line 4862 "lpc_parser.cpp"
    break;

  case 138: // arg_list: %empty
#line 977 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{}); }
#line 4868 "lpc_parser.cpp"
    break;

  case 139: // arg_list: arguments
#line 978 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4874 "lpc_parser.cpp"
    break;


#line 4878 "lpc_parser.cpp"

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
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
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

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

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
    error (yyexc.location, yyexc.what ());
  }

#if YYDEBUG || 0
  const char *
  LpcParser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytname_[yysymbol];
  }
#endif // #if YYDEBUG || 0









  const short LpcParser::yypact_ninf_ = -159;

  const signed char LpcParser::yytable_ninf_ = -127;

  const short
  LpcParser::yypact_[] =
  {
    -159,    43,     3,  -159,   -14,  -159,  -159,  -159,  -159,    38,
    -159,     5,    32,    67,  -159,  -159,     7,    58,    78,   -29,
    -159,   -14,  -159,  -159,  -159,  -159,    95,  -159,    80,  -159,
     -19,  -159,    38,     7,    50,  -159,  -159,   347,    96,  -159,
      84,    38,  -159,   141,   133,   143,  -159,    26,  -159,  -159,
    -159,   -12,   347,   347,   347,   -11,   -11,   347,   347,   347,
     182,  -159,   408,    62,    68,  -159,    65,  -159,   181,  -159,
     198,  -159,    41,    50,  -159,   182,   184,   179,  -159,  -159,
    -159,  -159,   -24,  -159,   408,    73,   265,    15,   146,   408,
     347,   347,   347,   347,   347,   347,   347,   347,   347,   347,
     347,   347,   347,   347,   347,   347,  -159,  -159,   207,   347,
     215,   380,  -159,  -159,  -159,  -159,  -159,  -159,   380,   347,
     190,   347,   347,   201,  -159,   347,   370,   279,   422,   436,
     117,   174,   231,   286,   286,   132,   132,  -159,  -159,  -159,
     408,   408,   213,    16,  -159,   -20,   408,  -159,  -159,   219,
     221,  -159,   220,   408,  -159,   408,   309,  -159,   408,   347,
     380,  -159,   347,   380,  -159,   180,  -159,   347,   408,   224,
     408,  -159,   226,   234,   237,   235,   238,   240,   262,   336,
     269,    81,   271,   237,  -159,  -159,    38,    21,  -159,  -159,
     237,    71,  -159,  -159,   123,  -159,   347,  -159,   123,  -159,
    -159,  -159,  -159,  -159,   408,  -159,   347,  -159,   328,  -159,
    -159,  -159,  -159,  -159,    82,   347,   274,  -159,   318,    89,
    -159,  -159,   108,  -159,   347,    -7,    57,   347,   163,   281,
    -159,   166,  -159,   282,  -159,  -159,   289,   284,   -11,   237,
     175,   237,   347,  -159,    50,   347,    46,  -159,   237,   352,
     203,   237,   210,   301,   237,  -159,   237,  -159,   304,  -159,
     237,   237,  -159,  -159,  -159,  -159,  -159
  };

  const unsigned char
  LpcParser::yydefact_[] =
  {
       3,     0,     2,     1,     0,    13,    15,    20,     6,    19,
      12,    11,     0,     0,     4,    29,    25,     5,     0,     0,
      21,     0,    14,    16,     8,    17,     0,    30,     0,     7,
      23,    10,    19,    25,    31,    26,    27,     0,     0,    22,
       0,    19,    37,     0,     0,    32,    33,     0,    93,    61,
      62,    94,     0,     0,     0,     0,     0,     0,     0,     0,
      63,    90,    24,     0,    89,    96,     0,    95,    23,    28,
      36,    38,     0,     0,   126,   129,   128,     0,    88,    86,
      87,    68,    91,    69,   120,     0,     0,     0,     0,    64,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,     0,
       0,   138,    35,    39,    41,     9,    40,    34,   138,     0,
       0,     0,     0,     0,    92,     0,    85,     0,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    84,    83,
      66,    67,     0,     0,   127,    94,   134,   135,   136,   139,
       0,    43,     0,   121,   124,   122,     0,   125,    65,     0,
     138,    97,     0,     0,   130,     0,   131,     0,    72,     0,
     133,   137,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,    42,    19,    11,    56,    44,
       0,     0,    48,    52,     0,    50,     0,    51,     0,    53,
      49,    54,    55,    47,   123,   132,     0,   111,     0,   107,
     109,    57,    58,   118,     0,     0,     0,    60,     0,     0,
      45,    46,     0,   103,   105,     0,     0,     0,     0,     0,
     119,     0,    59,     0,    99,   104,   106,     0,     0,     0,
       0,     0,     0,   115,     0,   105,     0,   112,     0,   101,
       0,     0,     0,     0,     0,   113,     0,   100,     0,   116,
       0,     0,   110,   102,   114,   117,   108
  };

  const short
  LpcParser::yypgoto_[] =
  {
    -159,  -159,  -159,  -159,  -159,  -159,   359,  -159,   364,  -159,
      12,   183,   338,   339,  -159,     1,  -159,   134,   307,  -159,
     305,  -159,  -159,  -158,  -159,   200,   -59,   -34,   126,   -54,
    -122,  -159,  -159,   191,   137,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,   344,
    -159,  -159,  -159,   239,  -159,  -111
  };

  const short
  LpcParser::yydefgoto_[] =
  {
       0,     1,     2,    17,     8,    34,   186,    10,   187,    12,
      38,    19,    20,    28,    14,    60,    44,    45,    46,   115,
     188,   151,   165,   189,   190,    61,   191,    89,    63,    64,
     192,   193,   257,   224,   237,   194,   195,   196,   197,   198,
     199,   200,   201,   251,   202,   203,    85,    87,    65,    66,
      77,    67,   147,   148,   149,   150
  };

  const short
  LpcParser::yytable_[] =
  {
      88,    82,    82,    62,   238,    16,     4,   152,    21,    47,
      48,    26,   162,    37,    13,    15,   208,    51,    78,    79,
      80,    18,    33,    84,    86,   218,     5,    31,    22,   108,
      32,   -19,   220,     6,  -126,    23,    27,  -126,   -18,   109,
      57,    58,  -126,     3,    22,  -126,    59,     7,    75,   169,
     143,    23,   125,    70,    74,    15,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   223,    41,   122,   125,   223,   146,    42,   123,
     161,   247,     7,   249,   146,   153,   104,   155,   156,    24,
     255,   158,   -91,   259,   105,    25,   262,   113,   263,   108,
     -91,   114,   265,   266,   254,    43,    30,    49,    50,   109,
      15,   106,   107,   235,    29,   239,   125,   -91,   -91,   110,
     214,   108,   111,    35,    68,   168,   146,   221,   170,   146,
     125,   109,   119,   204,   120,   222,    36,   225,   230,   226,
      69,   125,    90,    47,    48,   234,     5,   228,    32,    49,
      50,    51,    15,     6,    95,    96,   231,    97,    98,    99,
     100,   101,   102,   103,   235,   236,    52,   125,   240,    71,
      53,    54,    55,    56,    57,    58,   101,   102,   103,   184,
      59,    81,    83,   250,   246,   172,   236,   173,   174,   175,
     176,    72,   177,   178,   179,   180,   181,   182,   183,    90,
      47,    48,    73,     5,   124,   125,    49,    50,    51,    15,
       6,    27,    96,    37,    97,    98,    99,   100,   101,   102,
     103,   241,   125,    52,   243,   125,   112,    53,    54,    55,
      56,    57,    58,   248,   125,   142,   184,    59,   110,   118,
     114,   185,   172,   144,   173,   174,   175,   176,   154,   177,
     178,   179,   180,   181,   182,   183,    90,    47,    48,   157,
       5,   258,   125,    49,    50,    51,    15,     6,   260,    73,
     160,    97,    98,    99,   100,   101,   102,   103,   163,   164,
      52,   166,   205,   206,    53,    54,    55,    56,    57,    58,
      90,   207,   209,   184,    59,   210,   211,   114,    91,    92,
      93,    94,    95,    96,    90,    97,    98,    99,   100,   101,
     102,   103,    91,    92,    93,    94,    95,    96,   212,    97,
      98,    99,   100,   101,   102,   103,   215,   121,    99,   100,
     101,   102,   103,   217,    90,   229,   232,   233,   242,   244,
     245,   159,    91,    92,    93,    94,    95,    96,   125,    97,
      98,    99,   100,   101,   102,   103,    47,    48,   256,   261,
     264,     9,    49,    50,    51,    15,    11,    47,    48,   219,
      39,   167,    40,    49,    50,    51,    15,   116,   252,    52,
     117,   216,   253,    53,    54,    55,    56,    57,    58,   227,
      52,    76,   213,    59,    53,    54,    55,    56,    57,    58,
      47,    48,   171,     0,    59,     0,    49,    50,   145,    15,
      97,    98,    99,   100,   101,   102,   103,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,    53,    54,    55,
      56,    57,    58,    90,     0,     0,     0,    59,     0,     0,
       0,    91,    92,    93,    94,    95,    96,    90,    97,    98,
      99,   100,   101,   102,   103,     0,     0,    93,    94,    95,
      96,    90,    97,    98,    99,   100,   101,   102,   103,     0,
       0,     0,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103
  };

  const short
  LpcParser::yycheck_[] =
  {
      59,    55,    56,    37,    11,     4,     3,   118,     3,    20,
      21,     4,    32,    32,     2,    29,   174,    28,    52,    53,
      54,     9,    21,    57,    58,   183,    23,    56,    23,    53,
      59,    28,   190,    30,    54,    30,    29,    57,    57,    63,
      51,    52,    54,     0,    23,    57,    57,    44,    47,   160,
     109,    30,    59,    41,    28,    29,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   194,    23,    59,    59,   198,   111,    28,    64,
      64,   239,    44,   241,   118,   119,    24,   121,   122,    57,
     248,   125,    24,   251,    32,    28,   254,    56,   256,    53,
      32,    60,   260,   261,    58,    55,    28,    26,    27,    63,
      29,    49,    50,    56,    56,    58,    59,    49,    50,    54,
     179,    53,    57,    28,    28,   159,   160,    56,   162,   163,
      59,    63,    59,   167,    61,   194,    56,   196,    56,   198,
      56,    59,    25,    20,    21,    56,    23,   206,    59,    26,
      27,    28,    29,    30,    37,    38,   215,    40,    41,    42,
      43,    44,    45,    46,    56,   224,    43,    59,   227,    28,
      47,    48,    49,    50,    51,    52,    44,    45,    46,    56,
      57,    55,    56,   242,   238,     5,   245,     7,     8,     9,
      10,    58,    12,    13,    14,    15,    16,    17,    18,    25,
      20,    21,    59,    23,    58,    59,    26,    27,    28,    29,
      30,    29,    38,    32,    40,    41,    42,    43,    44,    45,
      46,    58,    59,    43,    58,    59,    28,    47,    48,    49,
      50,    51,    52,    58,    59,    28,    56,    57,    54,    60,
      60,    61,     5,    28,     7,     8,     9,    10,    58,    12,
      13,    14,    15,    16,    17,    18,    25,    20,    21,    58,
      23,    58,    59,    26,    27,    28,    29,    30,    58,    59,
      57,    40,    41,    42,    43,    44,    45,    46,    59,    58,
      43,    61,    58,    57,    47,    48,    49,    50,    51,    52,
      25,    57,    57,    56,    57,    57,    56,    60,    33,    34,
      35,    36,    37,    38,    25,    40,    41,    42,    43,    44,
      45,    46,    33,    34,    35,    36,    37,    38,    56,    40,
      41,    42,    43,    44,    45,    46,    57,    62,    42,    43,
      44,    45,    46,    62,    25,     7,    62,    19,    57,    57,
      56,    62,    33,    34,    35,    36,    37,    38,    59,    40,
      41,    42,    43,    44,    45,    46,    20,    21,     6,    58,
      56,     2,    26,    27,    28,    29,     2,    20,    21,   186,
      32,    62,    33,    26,    27,    28,    29,    72,   244,    43,
      73,   181,   245,    47,    48,    49,    50,    51,    52,   198,
      43,    47,    56,    57,    47,    48,    49,    50,    51,    52,
      20,    21,   163,    -1,    57,    -1,    26,    27,    28,    29,
      40,    41,    42,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    47,    48,    49,
      50,    51,    52,    25,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    33,    34,    35,    36,    37,    38,    25,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    35,    36,    37,
      38,    25,    40,    41,    42,    43,    44,    45,    46,    -1,
      -1,    -1,    36,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,    46
  };

  const signed char
  LpcParser::yystos_[] =
  {
       0,    66,    67,     0,     3,    23,    30,    44,    69,    71,
      72,    73,    74,    75,    79,    29,    80,    68,    75,    76,
      77,     3,    23,    30,    57,    28,     4,    29,    78,    56,
      28,    56,    59,    80,    70,    28,    56,    32,    75,    77,
      78,    23,    28,    55,    81,    82,    83,    20,    21,    26,
      27,    28,    43,    47,    48,    49,    50,    51,    52,    57,
      80,    90,    92,    93,    94,   113,   114,   116,    28,    56,
      75,    28,    58,    59,    28,    80,   114,   115,    92,    92,
      92,    93,    94,    93,    92,   111,    92,   112,    91,    92,
      25,    33,    34,    35,    36,    37,    38,    40,    41,    42,
      43,    44,    45,    46,    24,    32,    49,    50,    53,    63,
      54,    57,    28,    56,    60,    84,    85,    83,    60,    59,
      61,    62,    59,    64,    58,    59,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    28,    91,    28,    28,    92,   117,   118,   119,
     120,    86,   120,    92,    58,    92,    92,    58,    92,    62,
      57,    64,    32,    59,    58,    87,    61,    62,    92,   120,
      92,   118,     5,     7,     8,     9,    10,    12,    13,    14,
      15,    16,    17,    18,    56,    61,    71,    73,    85,    88,
      89,    91,    95,    96,   100,   101,   102,   103,   104,   105,
     106,   107,   109,   110,    92,    58,    57,    57,    88,    57,
      57,    56,    56,    56,    91,    57,    90,    62,    88,    76,
      88,    56,    91,    95,    98,    91,    91,    98,    91,     7,
      56,    91,    62,    19,    56,    56,    91,    99,    11,    58,
      91,    58,    57,    58,    57,    56,    94,    88,    58,    88,
      91,   108,    82,    99,    58,    88,     6,    97,    58,    88,
      58,    58,    88,    88,    56,    88,    88
  };

  const signed char
  LpcParser::yyr1_[] =
  {
       0,    65,    66,    67,    67,    67,    68,    68,    70,    69,
      69,    71,    71,    72,    72,    73,    73,    74,    74,    75,
      75,    76,    76,    77,    77,    78,    78,    79,    79,    80,
      80,    81,    81,    82,    82,    83,    83,    83,    83,    84,
      84,    86,    85,    87,    87,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    89,
      89,    90,    90,    90,    91,    91,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    93,    94,    94,    94,    94,    94,    94,    95,    95,
      96,    97,    97,    98,    98,    99,    99,   100,   101,   102,
     103,   104,   105,   105,   106,   108,   107,   109,   110,   110,
     111,   111,   112,   112,   113,   113,   114,   114,   115,   115,
     116,   116,   116,   117,   118,   118,   119,   119,   120,   120
  };

  const signed char
  LpcParser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     3,     0,     2,     0,     6,
       3,     1,     1,     1,     2,     1,     2,     2,     3,     0,
       1,     1,     3,     2,     4,     0,     2,     4,     5,     1,
       2,     0,     1,     1,     3,     3,     2,     1,     2,     1,
       1,     0,     4,     0,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     3,
       2,     1,     1,     1,     1,     3,     3,     3,     2,     2,
       2,     2,     5,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     1,
       1,     1,     3,     1,     1,     1,     1,     4,     1,     3,
       6,     0,     2,     1,     2,     0,     1,     2,     7,     2,
       6,     2,     4,     5,     7,     0,     6,     7,     2,     3,
       1,     3,     3,     5,     4,     4,     1,     3,     1,     1,
       4,     5,     6,     3,     1,     1,     1,     3,     0,     1
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const LpcParser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "L_INHERIT", "L_AS",
  "L_IF", "L_ELSE", "L_WHILE", "L_DO", "L_FOR", "L_FOREACH", "L_IN",
  "L_BREAK", "L_CONTINUE", "L_RETURN", "L_SWITCH", "L_CASE", "L_DEFAULT",
  "L_TRY", "L_CATCH", "L_NEW", "L_SELF", "LOWER_THAN_ELSE", "L_TYPE",
  "L_ASSIGN", "L_ORDER", "L_INTEGER", "L_REAL_NUMBER", "L_IDENTIFIER",
  "L_STRING_LITERAL", "L_TYPE_MODIFIER", "POSITIONAL_IDENTIFIER", "'='",
  "'?'", "L_LOR", "L_LAND", "'|'", "'^'", "'&'", "'<'", "L_LSH", "L_RSH",
  "'+'", "'-'", "'*'", "'%'", "'/'", "L_NOT", "'~'", "L_INC", "L_DEC",
  "L_ARRAY_OPEN", "L_MAPPING_OPEN", "L_ARROW", "L_COLON_COLON",
  "L_ELLIPSIS", "';'", "'('", "')'", "','", "'{'", "'}'", "':'", "'['",
  "']'", "$accept", "all", "program", "extra_semicolon", "def", "$@1",
  "storage_or_type", "typed_storage", "type_modifier_list",
  "function_head", "opt_star", "var_list", "new_var", "opt_as",
  "inheritance", "str_literal", "formal_parameters", "parameter_list",
  "new_param", "block_or_semicolon", "block", "$@2", "stmt_list", "stmt",
  "label", "const_expr", "comma_expr", "expr0", "lvalue", "expr4",
  "local_decl_list", "if_stmt", "opt_else_stmt", "loop_init",
  "opt_comma_expr", "for_begin", "for_stmt", "foreach_begin",
  "foreach_stmt", "while_begin", "while_stmt", "do_stmt", "switch_stmt",
  "$@3", "try_catch_stmt", "return_stmt", "expr_list", "assoc_list",
  "aggregation", "id", "class_or_name", "function_call", "kwarg",
  "argument", "arguments", "arg_list", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  LpcParser::yyrline_[] =
  {
       0,   182,   182,   186,   187,   193,   202,   203,   208,   207,
     246,   254,   259,   267,   269,   274,   275,   280,   289,   298,
     300,   305,   306,   325,   344,   370,   372,   377,   391,   408,
     410,   415,   417,   421,   423,   436,   449,   457,   469,   484,
     486,   491,   490,   522,   524,   536,   538,   539,   540,   541,
     542,   543,   544,   545,   546,   547,   548,   549,   551,   557,
     562,   570,   592,   597,   602,   603,   608,   610,   612,   614,
     616,   618,   620,   628,   630,   632,   634,   636,   638,   640,
     642,   644,   646,   648,   650,   652,   654,   656,   658,   660,
     661,   666,   670,   672,   674,   683,   684,   685,   689,   691,
     696,   707,   709,   714,   715,   719,   721,   725,   733,   760,
     768,   782,   790,   801,   828,   839,   838,   851,   862,   864,
     869,   874,   883,   890,   900,   902,   907,   909,   913,   915,
     920,   928,   935,   946,   954,   955,   959,   964,   976,   978
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
#line 5421 "lpc_parser.cpp"

#line 981 "src/lpc_parser.yy"


// User subroutines section
void yy::LpcParser::error(const location_type& location, const std::string &msg) {
    throw parse_error(msg, location, yyget_extra(yyscanner));
}
