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
      // mount
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
      // destruct_stmt
      // return_stmt
      // expr_list
      // assoc_list
      // aggregation
      // res_acquire
      // function_call
      // opt_init_list
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
      // defined_id
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
    L_MOUNT = 258,                 // L_MOUNT
    L_INHERIT = 259,               // L_INHERIT
    L_AS = 260,                    // L_AS
    L_USING = 261,                 // L_USING
    L_IF = 262,                    // L_IF
    L_ELSE = 263,                  // L_ELSE
    L_WHILE = 264,                 // L_WHILE
    L_DO = 265,                    // L_DO
    L_FOR = 266,                   // L_FOR
    L_FOREACH = 267,               // L_FOREACH
    L_IN = 268,                    // L_IN
    L_BREAK = 269,                 // L_BREAK
    L_CONTINUE = 270,              // L_CONTINUE
    L_RETURN = 271,                // L_RETURN
    L_SWITCH = 272,                // L_SWITCH
    L_CASE = 273,                  // L_CASE
    L_DEFAULT = 274,               // L_DEFAULT
    L_TRY = 275,                   // L_TRY
    L_CATCH = 276,                 // L_CATCH
    L_NEW = 277,                   // L_NEW
    L_DESTRUCT = 278,              // L_DESTRUCT
    L_SELF = 279,                  // L_SELF
    LOWER_THAN_ELSE = 280,         // LOWER_THAN_ELSE
    L_TYPE = 281,                  // L_TYPE
    L_ASSIGN = 282,                // L_ASSIGN
    L_ORDER = 283,                 // L_ORDER
    L_INTEGER = 284,               // L_INTEGER
    L_REAL_NUMBER = 285,           // L_REAL_NUMBER
    L_IDENTIFIER = 286,            // L_IDENTIFIER
    L_STRING_LITERAL = 287,        // L_STRING_LITERAL
    L_TYPE_MODIFIER = 288,         // L_TYPE_MODIFIER
    POSITIONAL_IDENTIFIER = 289,   // POSITIONAL_IDENTIFIER
    L_LOR = 290,                   // L_LOR
    L_LAND = 291,                  // L_LAND
    L_LSH = 292,                   // L_LSH
    L_RSH = 293,                   // L_RSH
    L_NOT = 294,                   // L_NOT
    L_INC = 295,                   // L_INC
    L_DEC = 296,                   // L_DEC
    L_ARRAY_OPEN = 297,            // L_ARRAY_OPEN
    L_MAPPING_OPEN = 298,          // L_MAPPING_OPEN
    L_ARROW = 299,                 // L_ARROW
    L_COLON_COLON = 300,           // L_COLON_COLON
    L_ELLIPSIS = 301               // L_ELLIPSIS
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
        YYNTOKENS = 69, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_L_MOUNT = 3,                           // L_MOUNT
        S_L_INHERIT = 4,                         // L_INHERIT
        S_L_AS = 5,                              // L_AS
        S_L_USING = 6,                           // L_USING
        S_L_IF = 7,                              // L_IF
        S_L_ELSE = 8,                            // L_ELSE
        S_L_WHILE = 9,                           // L_WHILE
        S_L_DO = 10,                             // L_DO
        S_L_FOR = 11,                            // L_FOR
        S_L_FOREACH = 12,                        // L_FOREACH
        S_L_IN = 13,                             // L_IN
        S_L_BREAK = 14,                          // L_BREAK
        S_L_CONTINUE = 15,                       // L_CONTINUE
        S_L_RETURN = 16,                         // L_RETURN
        S_L_SWITCH = 17,                         // L_SWITCH
        S_L_CASE = 18,                           // L_CASE
        S_L_DEFAULT = 19,                        // L_DEFAULT
        S_L_TRY = 20,                            // L_TRY
        S_L_CATCH = 21,                          // L_CATCH
        S_L_NEW = 22,                            // L_NEW
        S_L_DESTRUCT = 23,                       // L_DESTRUCT
        S_L_SELF = 24,                           // L_SELF
        S_LOWER_THAN_ELSE = 25,                  // LOWER_THAN_ELSE
        S_L_TYPE = 26,                           // L_TYPE
        S_L_ASSIGN = 27,                         // L_ASSIGN
        S_L_ORDER = 28,                          // L_ORDER
        S_L_INTEGER = 29,                        // L_INTEGER
        S_L_REAL_NUMBER = 30,                    // L_REAL_NUMBER
        S_L_IDENTIFIER = 31,                     // L_IDENTIFIER
        S_L_STRING_LITERAL = 32,                 // L_STRING_LITERAL
        S_L_TYPE_MODIFIER = 33,                  // L_TYPE_MODIFIER
        S_POSITIONAL_IDENTIFIER = 34,            // POSITIONAL_IDENTIFIER
        S_35_ = 35,                              // '='
        S_36_ = 36,                              // '?'
        S_L_LOR = 37,                            // L_LOR
        S_L_LAND = 38,                           // L_LAND
        S_39_ = 39,                              // '|'
        S_40_ = 40,                              // '^'
        S_41_ = 41,                              // '&'
        S_42_ = 42,                              // '<'
        S_L_LSH = 43,                            // L_LSH
        S_L_RSH = 44,                            // L_RSH
        S_45_ = 45,                              // '+'
        S_46_ = 46,                              // '-'
        S_47_ = 47,                              // '*'
        S_48_ = 48,                              // '%'
        S_49_ = 49,                              // '/'
        S_L_NOT = 50,                            // L_NOT
        S_51_ = 51,                              // '~'
        S_L_INC = 52,                            // L_INC
        S_L_DEC = 53,                            // L_DEC
        S_L_ARRAY_OPEN = 54,                     // L_ARRAY_OPEN
        S_L_MAPPING_OPEN = 55,                   // L_MAPPING_OPEN
        S_L_ARROW = 56,                          // L_ARROW
        S_L_COLON_COLON = 57,                    // L_COLON_COLON
        S_L_ELLIPSIS = 58,                       // L_ELLIPSIS
        S_59_ = 59,                              // ';'
        S_60_ = 60,                              // '('
        S_61_ = 61,                              // ')'
        S_62_ = 62,                              // ','
        S_63_ = 63,                              // '{'
        S_64_ = 64,                              // '}'
        S_65_ = 65,                              // ':'
        S_66_ = 66,                              // '['
        S_67_ = 67,                              // ']'
        S_68_ = 68,                              // '.'
        S_YYACCEPT = 69,                         // $accept
        S_all = 70,                              // all
        S_program = 71,                          // program
        S_extra_semicolon = 72,                  // extra_semicolon
        S_def = 73,                              // def
        S_74_1 = 74,                             // $@1
        S_storage_or_type = 75,                  // storage_or_type
        S_typed_storage = 76,                    // typed_storage
        S_type_modifier_list = 77,               // type_modifier_list
        S_function_head = 78,                    // function_head
        S_opt_star = 79,                         // opt_star
        S_var_list = 80,                         // var_list
        S_new_var = 81,                          // new_var
        S_opt_as = 82,                           // opt_as
        S_mount = 83,                            // mount
        S_inheritance = 84,                      // inheritance
        S_str_literal = 85,                      // str_literal
        S_formal_parameters = 86,                // formal_parameters
        S_parameter_list = 87,                   // parameter_list
        S_new_param = 88,                        // new_param
        S_block_or_semicolon = 89,               // block_or_semicolon
        S_block = 90,                            // block
        S_91_2 = 91,                             // $@2
        S_stmt_list = 92,                        // stmt_list
        S_stmt = 93,                             // stmt
        S_label = 94,                            // label
        S_const_expr = 95,                       // const_expr
        S_comma_expr = 96,                       // comma_expr
        S_expr0 = 97,                            // expr0
        S_lvalue = 98,                           // lvalue
        S_expr4 = 99,                            // expr4
        S_local_decl_list = 100,                 // local_decl_list
        S_if_stmt = 101,                         // if_stmt
        S_opt_else_stmt = 102,                   // opt_else_stmt
        S_loop_init = 103,                       // loop_init
        S_opt_comma_expr = 104,                  // opt_comma_expr
        S_for_begin = 105,                       // for_begin
        S_for_stmt = 106,                        // for_stmt
        S_foreach_begin = 107,                   // foreach_begin
        S_foreach_stmt = 108,                    // foreach_stmt
        S_while_begin = 109,                     // while_begin
        S_while_stmt = 110,                      // while_stmt
        S_do_stmt = 111,                         // do_stmt
        S_switch_stmt = 112,                     // switch_stmt
        S_113_3 = 113,                           // $@3
        S_try_catch_stmt = 114,                  // try_catch_stmt
        S_destruct_stmt = 115,                   // destruct_stmt
        S_return_stmt = 116,                     // return_stmt
        S_expr_list = 117,                       // expr_list
        S_assoc_list = 118,                      // assoc_list
        S_aggregation = 119,                     // aggregation
        S_defined_id = 120,                      // defined_id
        S_call_prefix = 121,                     // call_prefix
        S_res_acquire = 122,                     // res_acquire
        S_function_call = 123,                   // function_call
        S_opt_init_list = 124,                   // opt_init_list
        S_kwarg = 125,                           // kwarg
        S_argument = 126,                        // argument
        S_arguments = 127,                       // arguments
        S_arg_list = 128                         // arg_list
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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
      make_L_MOUNT (location_type l)
      {
        return symbol_type (token::L_MOUNT, std::move (l));
      }
#else
      static
      symbol_type
      make_L_MOUNT (const location_type& l)
      {
        return symbol_type (token::L_MOUNT, l);
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
      make_L_USING (location_type l)
      {
        return symbol_type (token::L_USING, std::move (l));
      }
#else
      static
      symbol_type
      make_L_USING (const location_type& l)
      {
        return symbol_type (token::L_USING, l);
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
      make_L_DESTRUCT (location_type l)
      {
        return symbol_type (token::L_DESTRUCT, std::move (l));
      }
#else
      static
      symbol_type
      make_L_DESTRUCT (const location_type& l)
      {
        return symbol_type (token::L_DESTRUCT, l);
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
    static const unsigned char yystos_[];

    // YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.
    static const unsigned char yyr1_[];

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
      yylast_ = 648,     ///< Last index in yytable_.
      yynnts_ = 60,  ///< Number of nonterminal symbols.
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
       2,     2,     2,     2,     2,     2,     2,    48,    41,     2,
      60,    61,    47,    45,    62,    46,    68,    49,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    65,    59,
      42,    35,     2,    36,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,    67,    40,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    63,    39,    64,    51,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      37,    38,    43,    44,    50,    52,    53,    54,    55,    56,
      57,    58
    };
    // Last valid token kind.
    const int code_max = 301;

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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
#line 2807 "lpc_parser.cpp"


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

#line 2866 "lpc_parser.cpp"






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
#line 2962 "lpc_parser.cpp"

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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
      case symbol_kind::S_mount: // mount
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
      case symbol_kind::S_destruct_stmt: // destruct_stmt
      case symbol_kind::S_return_stmt: // return_stmt
      case symbol_kind::S_expr_list: // expr_list
      case symbol_kind::S_assoc_list: // assoc_list
      case symbol_kind::S_aggregation: // aggregation
      case symbol_kind::S_res_acquire: // res_acquire
      case symbol_kind::S_function_call: // function_call
      case symbol_kind::S_opt_init_list: // opt_init_list
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
      case symbol_kind::S_defined_id: // defined_id
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
#line 183 "src/lpc_parser.yy"
            { yyget_extra(yyscanner)->root = std::make_shared<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ())); }
#line 3746 "lpc_parser.cpp"
    break;

  case 3: // program: %empty
#line 187 "src/lpc_parser.yy"
             { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Program{}); }
#line 3752 "lpc_parser.cpp"
    break;

  case 4: // program: program mount
#line 188 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 3758 "lpc_parser.cpp"
    break;

  case 5: // program: program inheritance
#line 190 "src/lpc_parser.yy"
        {
            yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > ();
            if (yystack_[0].value.as < ASTNode > ().node.index() != 0)
                std::get<AST::Program>(yylhs.value.as < ASTNode > ().node).defs.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 3768 "lpc_parser.cpp"
    break;

  case 6: // program: program def extra_semicolon
#line 196 "src/lpc_parser.yy"
        {
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
            if (yystack_[1].value.as < ASTNode > ().node.index() != 0)
                std::get<AST::Program>(yylhs.value.as < ASTNode > ().node).defs.push_back(std::move(yystack_[1].value.as < ASTNode > ()));
        }
#line 3778 "lpc_parser.cpp"
    break;

  case 8: // extra_semicolon: extra_semicolon ';'
#line 205 "src/lpc_parser.yy"
                          { /* ignore extra semicolon(s) between definitions */ }
#line 3784 "lpc_parser.cpp"
    break;

  case 9: // $@1: %empty
#line 210 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup(yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name) && yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->signature.b.is_const)
                throw parse_error("Function already defined: " + yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name, yyscanner);
            lpcc->current_scope->emplace(yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name, yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()); // add the function to the current scope
            lpcc->current_func = lpcc->consts->find_or_add(yystack_[1].value.as < std::shared_ptr<SymbolTable::Entry> > ()->name); // set the current function being parsed
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::FUNCTION, 0); // enter a new subscope for the function body
            lpcc->next_local = 0; // reset overlay base for the function scope
        }
#line 3798 "lpc_parser.cpp"
    break;

  case 10: // def: function_head '(' $@1 formal_parameters ')' block_or_semicolon
#line 220 "src/lpc_parser.yy"
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
#line 3831 "lpc_parser.cpp"
    break;

  case 11: // def: storage_or_type var_list ';'
#line 249 "src/lpc_parser.yy"
        {
            /* variable declarations */
            yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > ();
        }
#line 3840 "lpc_parser.cpp"
    break;

  case 12: // storage_or_type: type_modifier_list
#line 257 "src/lpc_parser.yy"
        {
            (yylhs.value.as < SymbolSignature > () = yystack_[0].value.as < SymbolSignature > ()).b.data_type = LpcDataType::T_INT;
            yyget_extra(yyscanner)->variable_signature = yylhs.value.as < SymbolSignature > ();
        }
#line 3849 "lpc_parser.cpp"
    break;

  case 13: // storage_or_type: typed_storage
#line 262 "src/lpc_parser.yy"
        {
            yylhs.value.as < SymbolSignature > () = yystack_[0].value.as < SymbolSignature > ();
            yyget_extra(yyscanner)->variable_signature = yylhs.value.as < SymbolSignature > ();
        }
#line 3858 "lpc_parser.cpp"
    break;

  case 14: // typed_storage: L_TYPE
#line 270 "src/lpc_parser.yy"
        { yylhs.value.as < SymbolSignature > () = SymbolSignature{}; yylhs.value.as < SymbolSignature > ().b.data_type = yystack_[0].value.as < int > (); }
#line 3864 "lpc_parser.cpp"
    break;

  case 15: // typed_storage: type_modifier_list L_TYPE
#line 272 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = yystack_[1].value.as < SymbolSignature > ()).b.data_type = yystack_[0].value.as < int > (); }
#line 3870 "lpc_parser.cpp"
    break;

  case 16: // type_modifier_list: L_TYPE_MODIFIER
#line 276 "src/lpc_parser.yy"
      { yylhs.value.as < SymbolSignature > () = yystack_[0].value.as < SymbolSignature > (); }
#line 3876 "lpc_parser.cpp"
    break;

  case 17: // type_modifier_list: type_modifier_list L_TYPE_MODIFIER
#line 278 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = yystack_[1].value.as < SymbolSignature > ()).value |= yystack_[0].value.as < SymbolSignature > ().value; }
#line 3882 "lpc_parser.cpp"
    break;

  case 18: // function_head: opt_star L_IDENTIFIER
#line 283 "src/lpc_parser.yy"
        {
            SymbolSignature sig{0};
            sig.b.data_type = LpcDataType::T_INT; // implicit function return type
            sig.b.is_function = 1; // mark as function
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), sig);
            if (!yystack_[1].value.as < SymbolSignature > ().b.is_array)
                yylhs.location.begin = yystack_[0].location.begin;
        }
#line 3895 "lpc_parser.cpp"
    break;

  case 19: // function_head: storage_or_type opt_star L_IDENTIFIER
#line 292 "src/lpc_parser.yy"
        {
            yystack_[2].value.as < SymbolSignature > ().value |= yystack_[1].value.as < SymbolSignature > ().value;
            yystack_[2].value.as < SymbolSignature > ().b.is_function = 1; // mark as function
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = std::make_shared<SymbolTable::Entry>(yystack_[0].value.as < std::string > (), yystack_[2].value.as < SymbolSignature > ());
        }
#line 3905 "lpc_parser.cpp"
    break;

  case 20: // opt_star: %empty
#line 301 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = SymbolSignature{}).value = 0; }
#line 3911 "lpc_parser.cpp"
    break;

  case 21: // opt_star: '*'
#line 303 "src/lpc_parser.yy"
        { (yylhs.value.as < SymbolSignature > () = SymbolSignature{}).b.is_array = 1; /* mark as array */ }
#line 3917 "lpc_parser.cpp"
    break;

  case 22: // var_list: new_var
#line 307 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 3923 "lpc_parser.cpp"
    break;

  case 23: // var_list: var_list ',' new_var
#line 309 "src/lpc_parser.yy"
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
#line 3943 "lpc_parser.cpp"
    break;

  case 24: // new_var: opt_star L_IDENTIFIER
#line 328 "src/lpc_parser.yy"
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
#line 3966 "lpc_parser.cpp"
    break;

  case 25: // new_var: opt_star L_IDENTIFIER '=' expr0
#line 347 "src/lpc_parser.yy"
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
#line 3993 "lpc_parser.cpp"
    break;

  case 26: // opt_as: %empty
#line 373 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = ""; }
#line 3999 "lpc_parser.cpp"
    break;

  case 27: // opt_as: L_AS L_IDENTIFIER
#line 375 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = yystack_[0].value.as < std::string > (); }
#line 4005 "lpc_parser.cpp"
    break;

  case 28: // mount: L_MOUNT str_literal opt_as ';'
#line 380 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            std::string alias{yystack_[1].value.as < std::string > ().empty() ? yystack_[2].value.as < std::string > () : yystack_[1].value.as < std::string > ()};
            if (!lpcc->current_scope->lookup(alias)) {
                auto entry = std::make_shared<SymbolTable::Entry>(alias, SymbolSignature{0}); // alias symbol
                entry->value = lpcc->current_scope->new_token(yystack_[2].value.as < std::string > (), SymbolTable::TokenType::STRING_LITERAL);
                lpcc->current_scope->emplace(alias, entry); // add the pseudo entry to current scope
                ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Mount{ entry });
            }
            else if (!yystack_[1].value.as < std::string > ().empty()) {
                throw parse_error("Mount descriptor already defined: " + alias, yyscanner);
            }
        }
#line 4023 "lpc_parser.cpp"
    break;

  case 29: // inheritance: L_INHERIT str_literal opt_as ';'
#line 397 "src/lpc_parser.yy"
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
                throw parse_error("Inheritance descriptor already defined: " + alias, yyscanner);
            }
        }
#line 4041 "lpc_parser.cpp"
    break;

  case 30: // inheritance: type_modifier_list L_INHERIT str_literal opt_as ';'
#line 411 "src/lpc_parser.yy"
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
#line 4059 "lpc_parser.cpp"
    break;

  case 31: // str_literal: L_STRING_LITERAL
#line 428 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = yystack_[0].value.as < std::string > (); }
#line 4065 "lpc_parser.cpp"
    break;

  case 32: // str_literal: str_literal L_STRING_LITERAL
#line 430 "src/lpc_parser.yy"
        { yylhs.value.as < std::string > () = yystack_[1].value.as < std::string > () + yystack_[0].value.as < std::string > (); /* compile-time string concatenation */ }
#line 4071 "lpc_parser.cpp"
    break;

  case 33: // formal_parameters: %empty
#line 435 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ParameterList{}); }
#line 4077 "lpc_parser.cpp"
    break;

  case 34: // formal_parameters: parameter_list
#line 436 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4083 "lpc_parser.cpp"
    break;

  case 35: // parameter_list: new_param
#line 441 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ParameterList{ std::vector<std::shared_ptr<SymbolTable::Entry>>{std::move(yystack_[0].value.as < std::shared_ptr<SymbolTable::Entry> > ())} }); }
#line 4089 "lpc_parser.cpp"
    break;

  case 36: // parameter_list: parameter_list ',' new_param
#line 443 "src/lpc_parser.yy"
        {
            auto& params = std::get<AST::ParameterList>(yystack_[2].value.as < ASTNode > ().node).parameters;
            if(params.back()->signature.b.data_type == LpcDataType::T_VOID)
                throw parse_error("Cannot have parameters after a void parameter", yyscanner);
            if(params.back()->signature.b.data_type == LpcDataType::T_VARARG)
                throw parse_error("Cannot have parameters after a vararg parameter", yyscanner);
            params.push_back(std::move(yystack_[0].value.as < std::shared_ptr<SymbolTable::Entry> > ()));
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
        }
#line 4103 "lpc_parser.cpp"
    break;

  case 37: // new_param: L_TYPE opt_star L_IDENTIFIER
#line 456 "src/lpc_parser.yy"
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
#line 4120 "lpc_parser.cpp"
    break;

  case 38: // new_param: L_TYPE opt_star
#line 469 "src/lpc_parser.yy"
        {
            SymbolSignature s = yystack_[0].value.as < SymbolSignature > ();
            s.b.data_type = yystack_[1].value.as < int > ();
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local++; // increment local variable counter for anonymous parameter
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = std::make_shared<SymbolTable::Entry>("", s);
        }
#line 4132 "lpc_parser.cpp"
    break;

  case 39: // new_param: L_IDENTIFIER
#line 477 "src/lpc_parser.yy"
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
#line 4148 "lpc_parser.cpp"
    break;

  case 40: // new_param: L_ELLIPSIS L_IDENTIFIER
#line 489 "src/lpc_parser.yy"
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
#line 4164 "lpc_parser.cpp"
    break;

  case 41: // block_or_semicolon: ';'
#line 504 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); }
#line 4170 "lpc_parser.cpp"
    break;

  case 42: // block_or_semicolon: block
#line 505 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4176 "lpc_parser.cpp"
    break;

  case 43: // $@2: %empty
#line 510 "src/lpc_parser.yy"
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
#line 4192 "lpc_parser.cpp"
    break;

  case 44: // block: '{' $@2 stmt_list '}'
#line 522 "src/lpc_parser.yy"
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
#line 4213 "lpc_parser.cpp"
    break;

  case 45: // stmt_list: %empty
#line 542 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{}); /* No-op */ }
#line 4219 "lpc_parser.cpp"
    break;

  case 46: // stmt_list: stmt_list stmt
#line 544 "src/lpc_parser.yy"
        {
            if (yystack_[0].value.as < ASTNode > ().node.index() != 0) {
                if (std::get<AST::List>(yystack_[1].value.as < ASTNode > ().node).elts.empty())
                    yystack_[1].value.as < ASTNode > ().src = yystack_[0].value.as < ASTNode > ().src;
                std::get<AST::List>(yystack_[1].value.as < ASTNode > ().node).elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            }
            yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > ();
        }
#line 4232 "lpc_parser.cpp"
    break;

  case 47: // stmt: label stmt
#line 556 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4238 "lpc_parser.cpp"
    break;

  case 48: // stmt: comma_expr ';'
#line 557 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4244 "lpc_parser.cpp"
    break;

  case 49: // stmt: L_USING '(' expr4 ')' stmt
#line 559 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4250 "lpc_parser.cpp"
    break;

  case 50: // stmt: mount
#line 560 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4256 "lpc_parser.cpp"
    break;

  case 51: // stmt: return_stmt
#line 561 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4262 "lpc_parser.cpp"
    break;

  case 52: // stmt: local_decl_list
#line 562 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4268 "lpc_parser.cpp"
    break;

  case 53: // stmt: destruct_stmt
#line 563 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4274 "lpc_parser.cpp"
    break;

  case 54: // stmt: do_stmt
#line 564 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4280 "lpc_parser.cpp"
    break;

  case 55: // stmt: for_stmt
#line 565 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4286 "lpc_parser.cpp"
    break;

  case 56: // stmt: foreach_stmt
#line 566 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4292 "lpc_parser.cpp"
    break;

  case 57: // stmt: if_stmt
#line 567 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4298 "lpc_parser.cpp"
    break;

  case 58: // stmt: while_stmt
#line 568 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4304 "lpc_parser.cpp"
    break;

  case 59: // stmt: switch_stmt
#line 569 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4310 "lpc_parser.cpp"
    break;

  case 60: // stmt: try_catch_stmt
#line 570 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4316 "lpc_parser.cpp"
    break;

  case 61: // stmt: block
#line 571 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4322 "lpc_parser.cpp"
    break;

  case 62: // stmt: L_BREAK ';'
#line 573 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Break{}); }
#line 4328 "lpc_parser.cpp"
    break;

  case 63: // stmt: L_CONTINUE ';'
#line 575 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Continue{}); }
#line 4334 "lpc_parser.cpp"
    break;

  case 64: // label: L_CASE const_expr ':'
#line 581 "src/lpc_parser.yy"
        {
            if (yyget_extra(yyscanner)->switch_depth == 0)
                throw parse_error("case label outside switch", yystack_[2].location, yyget_extra(yyscanner));
        }
#line 4343 "lpc_parser.cpp"
    break;

  case 65: // label: L_DEFAULT ':'
#line 586 "src/lpc_parser.yy"
        {
            if (yyget_extra(yyscanner)->switch_depth == 0)
                throw parse_error("default label outside switch", yystack_[1].location, yyget_extra(yyscanner));
        }
#line 4352 "lpc_parser.cpp"
    break;

  case 66: // const_expr: L_INTEGER
#line 594 "src/lpc_parser.yy"
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
#line 4378 "lpc_parser.cpp"
    break;

  case 67: // const_expr: L_REAL_NUMBER
#line 616 "src/lpc_parser.yy"
        {
            auto consts = yyget_extra(yyscanner)->consts;
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::NumericConst{ yystack_[0].value.as < double > (), consts->find_or_add(yystack_[0].value.as < double > ()) });
        }
#line 4387 "lpc_parser.cpp"
    break;

  case 68: // const_expr: str_literal
#line 621 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::StringLiteral{yystack_[0].value.as < std::string > (), *yyget_extra(yyscanner)->consts}); }
#line 4393 "lpc_parser.cpp"
    break;

  case 69: // comma_expr: expr0
#line 625 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4399 "lpc_parser.cpp"
    break;

  case 70: // comma_expr: comma_expr ',' expr0
#line 627 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::COMMA, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4405 "lpc_parser.cpp"
    break;

  case 71: // expr0: lvalue L_ASSIGN expr0
#line 632 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{yystack_[1].value.as < int > (), std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4411 "lpc_parser.cpp"
    break;

  case 72: // expr0: lvalue '=' expr0
#line 634 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4417 "lpc_parser.cpp"
    break;

  case 73: // expr0: L_INC lvalue
#line 636 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_PREFIX_INC, nullptr, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4423 "lpc_parser.cpp"
    break;

  case 74: // expr0: L_DEC lvalue
#line 638 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_PREFIX_DEC, nullptr, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4429 "lpc_parser.cpp"
    break;

  case 75: // expr0: lvalue L_INC
#line 640 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_SUFFIX_INC, std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())), nullptr}); }
#line 4435 "lpc_parser.cpp"
    break;

  case 76: // expr0: lvalue L_DEC
#line 642 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Assign{oparg::ASSIGN_SUFFIX_DEC, std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())), nullptr}); }
#line 4441 "lpc_parser.cpp"
    break;

  case 77: // expr0: expr0 '?' expr0 ':' expr0
#line 644 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::TernaryOp{
                std::make_unique<ASTNode>(std::move(yystack_[4].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}
            );
        }
#line 4453 "lpc_parser.cpp"
    break;

  case 78: // expr0: expr0 L_LOR expr0
#line 652 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::LOGICAL_OR, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4459 "lpc_parser.cpp"
    break;

  case 79: // expr0: expr0 L_LAND expr0
#line 654 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::LOGICAL_AND, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4465 "lpc_parser.cpp"
    break;

  case 80: // expr0: expr0 '|' expr0
#line 656 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::BITWISE_OR, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4471 "lpc_parser.cpp"
    break;

  case 81: // expr0: expr0 '^' expr0
#line 658 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::BITWISE_XOR, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4477 "lpc_parser.cpp"
    break;

  case 82: // expr0: expr0 '&' expr0
#line 660 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::BITWISE_AND, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4483 "lpc_parser.cpp"
    break;

  case 83: // expr0: expr0 L_LSH expr0
#line 662 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::LSH, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4489 "lpc_parser.cpp"
    break;

  case 84: // expr0: expr0 L_RSH expr0
#line 664 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::RSH, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4495 "lpc_parser.cpp"
    break;

  case 85: // expr0: expr0 '+' expr0
#line 666 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::ADD, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4501 "lpc_parser.cpp"
    break;

  case 86: // expr0: expr0 '-' expr0
#line 668 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::SUB, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4507 "lpc_parser.cpp"
    break;

  case 87: // expr0: expr0 '*' expr0
#line 670 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::MUL, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4513 "lpc_parser.cpp"
    break;

  case 88: // expr0: expr0 '/' expr0
#line 672 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::DIV, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4519 "lpc_parser.cpp"
    break;

  case 89: // expr0: expr0 '%' expr0
#line 674 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{oparg::MOD, std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4525 "lpc_parser.cpp"
    break;

  case 90: // expr0: expr0 L_ORDER expr0
#line 676 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::BinaryOp{yystack_[1].value.as < int > (), std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4531 "lpc_parser.cpp"
    break;

  case 91: // expr0: L_NOT expr0
#line 678 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::UnaryOp{oparg::NOT, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4537 "lpc_parser.cpp"
    break;

  case 92: // expr0: '~' expr0
#line 680 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::UnaryOp{oparg::BITWISE_NOT, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4543 "lpc_parser.cpp"
    break;

  case 93: // expr0: '-' expr0
#line 682 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::UnaryOp{oparg::NEG, std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))}); }
#line 4549 "lpc_parser.cpp"
    break;

  case 94: // expr0: expr4
#line 683 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4555 "lpc_parser.cpp"
    break;

  case 95: // expr0: const_expr
#line 684 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4561 "lpc_parser.cpp"
    break;

  case 96: // expr0: aggregation
#line 685 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4567 "lpc_parser.cpp"
    break;

  case 97: // lvalue: expr4
#line 690 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4573 "lpc_parser.cpp"
    break;

  case 98: // expr4: '(' comma_expr ')'
#line 695 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4579 "lpc_parser.cpp"
    break;

  case 99: // expr4: L_SELF
#line 697 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Self{ }); }
#line 4585 "lpc_parser.cpp"
    break;

  case 100: // expr4: defined_id
#line 699 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Variable{ yystack_[0].value.as < std::shared_ptr<SymbolTable::Entry> > () }); }
#line 4591 "lpc_parser.cpp"
    break;

  case 101: // expr4: function_call
#line 700 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4597 "lpc_parser.cpp"
    break;

  case 102: // expr4: expr4 '[' comma_expr ']'
#line 702 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Index{std::make_unique<ASTNode>(std::move(yystack_[3].value.as < ASTNode > ())), std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))}); }
#line 4603 "lpc_parser.cpp"
    break;

  case 103: // local_decl_list: ';'
#line 707 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); /* i.e. No-op */ }
#line 4609 "lpc_parser.cpp"
    break;

  case 104: // local_decl_list: storage_or_type var_list ';'
#line 709 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4615 "lpc_parser.cpp"
    break;

  case 105: // if_stmt: L_IF '(' comma_expr ')' stmt opt_else_stmt
#line 714 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Cond{
                std::make_unique<ASTNode>(std::move(yystack_[3].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4627 "lpc_parser.cpp"
    break;

  case 106: // opt_else_stmt: %empty
#line 725 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); }
#line 4633 "lpc_parser.cpp"
    break;

  case 107: // opt_else_stmt: L_ELSE stmt
#line 727 "src/lpc_parser.yy"
        { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > ();}
#line 4639 "lpc_parser.cpp"
    break;

  case 108: // loop_init: local_decl_list
#line 731 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4645 "lpc_parser.cpp"
    break;

  case 109: // loop_init: comma_expr ';'
#line 732 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[1].value.as < ASTNode > (); }
#line 4651 "lpc_parser.cpp"
    break;

  case 110: // opt_comma_expr: %empty
#line 737 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, std::monostate{}); }
#line 4657 "lpc_parser.cpp"
    break;

  case 111: // opt_comma_expr: comma_expr
#line 738 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 4663 "lpc_parser.cpp"
    break;

  case 112: // for_begin: L_FOR '('
#line 743 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local);
        }
#line 4672 "lpc_parser.cpp"
    break;

  case 113: // for_stmt: for_begin loop_init opt_comma_expr ';' opt_comma_expr ')' stmt
#line 751 "src/lpc_parser.yy"
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
#line 4700 "lpc_parser.cpp"
    break;

  case 114: // foreach_begin: L_FOREACH '('
#line 778 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local);
        }
#line 4709 "lpc_parser.cpp"
    break;

  case 115: // foreach_stmt: foreach_begin comma_expr L_IN expr4 ')' stmt
#line 786 "src/lpc_parser.yy"
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
#line 4724 "lpc_parser.cpp"
    break;

  case 116: // while_begin: L_WHILE '('
#line 800 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local);
        }
#line 4733 "lpc_parser.cpp"
    break;

  case 117: // while_stmt: while_begin comma_expr ')' stmt
#line 808 "src/lpc_parser.yy"
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
#line 4748 "lpc_parser.cpp"
    break;

  case 118: // while_stmt: while_begin loop_init comma_expr ')' stmt
#line 819 "src/lpc_parser.yy"
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
#line 4776 "lpc_parser.cpp"
    break;

  case 119: // do_stmt: L_DO stmt L_WHILE '(' comma_expr ')' ';'
#line 846 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::PostTestedLoop{
                std::make_unique<ASTNode>(std::move(yystack_[5].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ()))
            });
        }
#line 4787 "lpc_parser.cpp"
    break;

  case 120: // $@3: %empty
#line 856 "src/lpc_parser.yy"
        { ++yyget_extra(yyscanner)->switch_depth; }
#line 4793 "lpc_parser.cpp"
    break;

  case 121: // switch_stmt: L_SWITCH '(' comma_expr ')' $@3 stmt
#line 858 "src/lpc_parser.yy"
        {
            --yyget_extra(yyscanner)->switch_depth;
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Switch{
                std::make_unique<ASTNode>(std::move(yystack_[3].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4805 "lpc_parser.cpp"
    break;

  case 122: // try_catch_stmt: L_TRY stmt L_CATCH '(' parameter_list ')' stmt
#line 869 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::TryCatch{
                std::make_unique<ASTNode>(std::move(yystack_[5].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[2].value.as < ASTNode > ())),
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))
            });
        }
#line 4817 "lpc_parser.cpp"
    break;

  case 123: // destruct_stmt: L_DESTRUCT lvalue ';'
#line 880 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Destruct{
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))
            });
        }
#line 4827 "lpc_parser.cpp"
    break;

  case 124: // return_stmt: L_RETURN ';'
#line 889 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Return{nullptr}); }
#line 4833 "lpc_parser.cpp"
    break;

  case 125: // return_stmt: L_RETURN comma_expr ';'
#line 891 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::Return{std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))}); }
#line 4839 "lpc_parser.cpp"
    break;

  case 126: // expr_list: expr0
#line 896 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{});
            std::get<AST::List>(yylhs.value.as < ASTNode > ().node).elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 4848 "lpc_parser.cpp"
    break;

  case 127: // expr_list: expr_list ',' expr0
#line 901 "src/lpc_parser.yy"
        {
            auto &elts = std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts;
            elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
        }
#line 4858 "lpc_parser.cpp"
    break;

  case 128: // assoc_list: expr0 ':' expr0
#line 910 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{});
            auto& elts = std::get<AST::List>(yylhs.value.as < ASTNode > ().node).elts;
            elts.push_back(std::move(yystack_[2].value.as < ASTNode > ()));
            elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 4869 "lpc_parser.cpp"
    break;

  case 129: // assoc_list: assoc_list ',' expr0 ':' expr0
#line 917 "src/lpc_parser.yy"
        {
            auto &elts = std::get<AST::List>(yystack_[4].value.as < ASTNode > ().node).elts;
            elts.push_back(std::move(yystack_[2].value.as < ASTNode > ()));
            elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            yylhs.value.as < ASTNode > () = yystack_[4].value.as < ASTNode > ();
        }
#line 4880 "lpc_parser.cpp"
    break;

  case 130: // aggregation: L_ARRAY_OPEN '}' ')'
#line 927 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ArrayLiteral{}); }
#line 4886 "lpc_parser.cpp"
    break;

  case 131: // aggregation: L_ARRAY_OPEN expr_list '}' ')'
#line 929 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ArrayLiteral{ std::move(std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts) }); }
#line 4892 "lpc_parser.cpp"
    break;

  case 132: // aggregation: L_MAPPING_OPEN assoc_list ']' ')'
#line 931 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::MappingLiteral{ std::move(std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts) }); }
#line 4898 "lpc_parser.cpp"
    break;

  case 133: // aggregation: L_MAPPING_OPEN ']' ')'
#line 933 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::MappingLiteral{}); }
#line 4904 "lpc_parser.cpp"
    break;

  case 134: // defined_id: L_IDENTIFIER
#line 939 "src/lpc_parser.yy"
        {
            auto entry = yyget_extra(yyscanner)->current_scope->lookup(yystack_[0].value.as < std::string > ());
            if (!entry) {
                throw parse_error("Undefined identifier: " + std::string(yystack_[0].value.as < std::string > ()), yylhs.location, yyget_extra(yyscanner));
            }
            yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = entry;
        }
#line 4916 "lpc_parser.cpp"
    break;

  case 135: // defined_id: defined_id '.' L_IDENTIFIER
#line 946 "src/lpc_parser.yy"
      { yylhs.value.as < std::shared_ptr<SymbolTable::Entry> > () = yystack_[2].value.as < std::shared_ptr<SymbolTable::Entry> > (); }
#line 4922 "lpc_parser.cpp"
    break;

  case 139: // res_acquire: defined_id L_COLON_COLON L_IDENTIFIER
#line 957 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ResourceAcquire{ std::move(yystack_[2].value.as < std::shared_ptr<SymbolTable::Entry> > ()), yyget_extra(yyscanner)->consts->find_or_add(yystack_[0].value.as < std::string > ()) }); }
#line 4928 "lpc_parser.cpp"
    break;

  case 140: // res_acquire: call_prefix L_IDENTIFIER
#line 959 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ResourceAcquire{ nullptr, yyget_extra(yyscanner)->current_scope->lookup(yystack_[0].value.as < std::string > ()) }); }
#line 4934 "lpc_parser.cpp"
    break;

  case 141: // function_call: defined_id '(' arg_list ')'
#line 964 "src/lpc_parser.yy"
        {
            auto lpcc = yyget_extra(yyscanner);
            if (!yystack_[3].value.as < std::shared_ptr<SymbolTable::Entry> > ())
                throw parse_error("Function not found", yylhs.location, yyget_extra(yyscanner));
            yystack_[3].value.as < std::shared_ptr<SymbolTable::Entry> > ()->signature.b.referenced = 1; // mark the function as referenced
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::FunctionCall{ std::move(yystack_[3].value.as < std::shared_ptr<SymbolTable::Entry> > ()), std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())) });
        }
#line 4946 "lpc_parser.cpp"
    break;

  case 142: // function_call: L_NEW res_acquire opt_init_list
#line 972 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::New{
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ())), // clone semantics for the resource
                std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()))  // initialization list for globals
            });
        }
#line 4957 "lpc_parser.cpp"
    break;

  case 143: // function_call: res_acquire '(' arg_list ')'
#line 979 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::ResourceCall{
                std::make_unique<ASTNode>(std::move(yystack_[3].value.as < ASTNode > ())), // find-or-load semantic for the resource
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))
            });
        }
#line 4968 "lpc_parser.cpp"
    break;

  case 144: // function_call: expr4 L_ARROW L_IDENTIFIER '(' arg_list ')'
#line 986 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::CallOther{
                std::make_unique<ASTNode>(std::move(yystack_[5].value.as < ASTNode > ())), // dereference the object to call the method on
                yyget_extra(yyscanner)->consts->find_or_add(yystack_[3].value.as < std::string > ()),
                std::make_unique<ASTNode>(std::move(yystack_[1].value.as < ASTNode > ()))
            });
        }
#line 4980 "lpc_parser.cpp"
    break;

  case 145: // opt_init_list: %empty
#line 997 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{}); }
#line 4986 "lpc_parser.cpp"
    break;

  case 146: // opt_init_list: '{' arg_list '}'
#line 999 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{ std::move(std::get<AST::List>(yystack_[1].value.as < ASTNode > ().node).elts) }); }
#line 4992 "lpc_parser.cpp"
    break;

  case 147: // kwarg: L_IDENTIFIER '=' expr0
#line 1004 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::KeywordArgument{ yystack_[2].value.as < std::string > (), *yyget_extra(yyscanner)->consts });
            std::get<AST::KeywordArgument>(yylhs.value.as < ASTNode > ().node).value = std::make_unique<ASTNode>(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 5001 "lpc_parser.cpp"
    break;

  case 148: // argument: expr0
#line 1011 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 5007 "lpc_parser.cpp"
    break;

  case 149: // argument: kwarg
#line 1012 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 5013 "lpc_parser.cpp"
    break;

  case 150: // arguments: argument
#line 1017 "src/lpc_parser.yy"
        {
            ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{});
            std::get<AST::List>(yylhs.value.as < ASTNode > ().node).elts.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
        }
#line 5022 "lpc_parser.cpp"
    break;

  case 151: // arguments: arguments ',' argument
#line 1022 "src/lpc_parser.yy"
        {
            auto& args = std::get<AST::List>(yystack_[2].value.as < ASTNode > ().node).elts;
            if (std::holds_alternative<AST::KeywordArgument>(args.back().node)
                && !std::holds_alternative<AST::KeywordArgument>(yystack_[0].value.as < ASTNode > ().node))
                throw parse_error("Positional argument after keyword argument", yystack_[0].location, yyget_extra(yyscanner));
            args.push_back(std::move(yystack_[0].value.as < ASTNode > ()));
            yylhs.value.as < ASTNode > () = yystack_[2].value.as < ASTNode > ();
        }
#line 5035 "lpc_parser.cpp"
    break;

  case 152: // arg_list: %empty
#line 1034 "src/lpc_parser.yy"
        { ASSIGN_AST(yylhs.value.as < ASTNode > (), yylhs.location, AST::List{}); }
#line 5041 "lpc_parser.cpp"
    break;

  case 153: // arg_list: arguments
#line 1035 "src/lpc_parser.yy"
      { yylhs.value.as < ASTNode > () = yystack_[0].value.as < ASTNode > (); }
#line 5047 "lpc_parser.cpp"
    break;


#line 5051 "lpc_parser.cpp"

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









  const short LpcParser::yypact_ninf_ = -173;

  const signed char LpcParser::yytable_ninf_ = -98;

  const short
  LpcParser::yypact_[] =
  {
    -173,     9,     8,  -173,   -17,   -17,  -173,  -173,  -173,  -173,
       0,  -173,    24,    19,    55,  -173,  -173,  -173,    20,    20,
      53,    92,    66,  -173,   -17,  -173,  -173,  -173,  -173,    93,
    -173,    70,    76,  -173,    -9,  -173,     0,    20,   -10,  -173,
    -173,  -173,   466,   113,  -173,    90,     0,  -173,   120,    32,
     116,  -173,    59,  -173,  -173,  -173,  -173,   466,   466,   466,
     -14,   -14,   345,   386,  -173,  -173,   466,  -173,   135,  -173,
     151,   105,    54,  -173,    62,   124,   122,  -173,   158,  -173,
     172,  -173,   -26,   -10,   -30,   147,  -173,  -173,  -173,  -173,
     -21,  -173,   154,   151,    39,   166,   539,    38,    -3,   151,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   466,   466,   466,   466,   466,  -173,  -173,   206,   466,
     216,   506,   218,  -173,   506,  -173,  -173,  -173,  -173,  -173,
    -173,   506,  -173,  -173,   466,   189,  -173,   466,   466,   190,
    -173,   466,   296,   553,   340,   421,   379,   195,   258,   114,
     114,   128,   128,  -173,  -173,  -173,   151,   151,   200,    47,
    -173,   228,   151,  -173,  -173,   205,   203,  -173,   208,  -173,
     209,   151,  -173,   151,   583,  -173,   151,   466,   506,  -173,
     466,   506,  -173,  -173,   202,  -173,   466,   151,   217,   151,
    -173,   230,   232,   233,   265,   239,   240,   249,   250,   426,
     252,   136,   245,   265,   -14,  -173,  -173,     0,    51,  -173,
    -173,  -173,   265,    84,  -173,  -173,   305,  -173,   466,  -173,
     305,  -173,  -173,  -173,  -173,  -173,  -173,   151,  -173,   -14,
     466,  -173,   304,  -173,  -173,  -173,  -173,  -173,    94,   466,
     261,  -173,   293,   264,   110,  -173,  -173,   112,  -173,   466,
      -6,    75,   466,    22,   119,   270,  -173,   123,  -173,   272,
    -173,  -173,  -173,   284,   288,   -14,   265,   140,   265,   265,
     466,  -173,   -10,   466,    65,  -173,   265,  -173,   341,   168,
     265,   184,   287,   265,  -173,   265,  -173,   291,  -173,   265,
     265,  -173,  -173,  -173,  -173,  -173
  };

  const unsigned char
  LpcParser::yydefact_[] =
  {
       3,     0,     2,     1,     0,     0,    14,    16,    21,     7,
      20,    13,    12,     0,     0,     4,     5,    31,    26,    26,
       6,     0,     0,    22,     0,    15,    17,     9,    18,     0,
      32,     0,     0,     8,    24,    11,    20,    26,    33,    27,
      28,    29,     0,     0,    23,     0,    20,    39,     0,     0,
      34,    35,     0,    99,    66,    67,   134,     0,     0,     0,
       0,     0,     0,     0,   137,   138,     0,   136,    68,    95,
      25,     0,    94,    96,   100,     0,     0,   101,    24,    30,
      38,    40,     0,     0,     0,   145,    93,    91,    92,    73,
      97,    74,     0,   126,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,    76,     0,     0,
       0,   152,     0,   140,   152,    37,    41,    43,    10,    42,
      36,   152,   142,   130,     0,     0,   133,     0,     0,     0,
      98,     0,    90,     0,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    89,    88,    71,    72,     0,     0,
     139,   134,   148,   149,   150,   153,     0,   135,     0,    45,
       0,   127,   131,   128,     0,   132,    70,     0,   152,   102,
       0,     0,   141,   143,     0,   146,     0,    77,     0,   147,
     151,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   103,    44,    20,    12,    50,
      61,    46,     0,     0,    52,    57,     0,    55,     0,    56,
       0,    58,    54,    59,    60,    53,    51,   129,   144,     0,
       0,   116,     0,   112,   114,    62,    63,   124,     0,     0,
       0,    65,     0,     0,     0,    47,    48,     0,   108,   110,
       0,     0,     0,     0,     0,     0,   125,     0,    64,     0,
     123,   104,   109,   111,     0,     0,     0,     0,     0,     0,
       0,   120,     0,   110,     0,   117,     0,    49,   106,     0,
       0,     0,     0,     0,   118,     0,   105,     0,   121,     0,
       0,   115,   107,   119,   122,   113
  };

  const short
  LpcParser::yypgoto_[] =
  {
    -173,  -173,  -173,  -173,  -173,  -173,   350,  -173,   351,  -173,
      34,   156,   318,    -5,   364,  -173,    25,  -173,    98,   289,
    -173,   300,  -173,  -173,  -172,  -173,   170,   -66,   -39,   -56,
     -59,  -129,  -173,  -173,   173,   117,  -173,  -173,  -173,  -173,
    -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,
    -173,   342,  -173,   352,  -173,  -173,  -173,   211,  -173,  -118
  };

  const short
  LpcParser::yydefgoto_[] =
  {
       0,     1,     2,    20,     9,    38,   207,    11,   208,    13,
      43,    22,    23,    31,   209,    16,    68,    49,    50,    51,
     128,   210,   169,   184,   211,   212,    69,   213,    99,    71,
      72,   214,   215,   286,   249,   264,   216,   217,   218,   219,
     220,   221,   222,   223,   280,   224,   225,   226,    94,    97,
      73,    74,    75,    76,    77,   132,   163,   164,   165,   166
  };

  const short
  LpcParser::yytable_[] =
  {
      98,    90,    90,    70,    89,    91,   168,   265,    52,     3,
      53,     4,     5,   170,    32,    17,    46,    56,    86,    87,
      88,    47,   232,    93,    96,    29,    42,   120,    24,    18,
      19,   242,    45,   126,     6,   118,    14,   127,   122,   -20,
     245,     7,    64,    65,    21,   119,    66,     8,    48,    37,
      25,   -19,    30,   159,    67,     8,   141,    26,   140,   141,
     188,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,    25,   118,    27,
      80,   -97,   162,   268,    26,   162,    28,   248,   119,   -97,
      56,   248,   162,    82,   275,   171,   277,   278,   173,   174,
     138,   134,   176,   135,   284,   139,   -97,   -97,   288,   141,
     118,   291,    33,   292,   179,    64,    65,   294,   295,   120,
     119,   118,   121,    34,    39,    35,   283,    67,    36,    40,
     122,   119,   114,   238,   262,    41,   266,   141,   187,   162,
     115,   189,   162,   246,    78,    90,   141,   227,   243,    79,
     247,    81,   250,   256,   251,   123,   141,   116,   117,   109,
     110,   111,   112,   113,   254,    54,    55,    30,    17,   261,
     253,   262,    36,   257,   141,   111,   112,   113,    83,   100,
     269,   141,   124,   263,   271,   141,   267,   101,   102,   103,
     104,   105,   106,    42,   107,   108,   109,   110,   111,   112,
     113,   276,   141,   125,   279,     4,   274,   263,   191,   192,
     131,   193,   194,   195,   196,   133,   197,   198,   199,   200,
     201,   202,   203,   100,    52,   204,    53,   136,     6,   287,
     141,    54,    55,    56,    17,     7,   106,   158,   107,   108,
     109,   110,   111,   112,   113,   289,    83,   160,    57,   167,
     172,   175,    58,    59,    60,    61,    62,    63,    64,    65,
     178,   205,    66,   180,   182,   127,   206,   181,     4,   183,
      67,   191,   192,   185,   193,   194,   195,   196,   228,   197,
     198,   199,   200,   201,   202,   203,   100,    52,   204,    53,
     229,     6,   230,   231,    54,    55,    56,    17,     7,   233,
     234,   107,   108,   109,   110,   111,   112,   113,   235,   236,
     241,    57,   239,   255,   259,    58,    59,    60,    61,    62,
      63,    64,    65,   260,   205,    66,   258,    52,   127,    53,
     270,     6,   272,    67,    54,    55,    56,    17,     7,   107,
     108,   109,   110,   111,   112,   113,   141,   273,   290,   285,
     293,    57,    10,    12,    44,    58,    59,    60,    61,    62,
      63,    64,    65,   244,   205,    66,    15,    52,   100,    53,
     281,   240,   130,    67,    54,    55,    56,    17,   103,   104,
     105,   106,   129,   107,   108,   109,   110,   111,   112,   113,
     282,    57,   190,   252,    84,    58,    59,    60,    61,    62,
      63,    64,    65,     0,    85,    66,     0,   100,    52,    92,
      53,     0,     0,    67,     0,    54,    55,    56,    17,   105,
     106,     0,   107,   108,   109,   110,   111,   112,   113,     0,
       0,     0,    57,     0,     0,     0,    58,    59,    60,    61,
      62,    63,    64,    65,     0,     0,    66,     0,    52,   100,
      53,     0,     0,    95,    67,    54,    55,    56,    17,     0,
     104,   105,   106,     0,   107,   108,   109,   110,   111,   112,
     113,     0,    57,     0,     0,     0,    58,    59,    60,    61,
      62,    63,    64,    65,     0,   237,    66,     0,    52,     0,
      53,     0,     0,     0,    67,    54,    55,    56,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    57,     0,     0,     0,    58,    59,    60,    61,
      62,    63,    64,    65,     0,     0,    66,     0,    52,     0,
      53,     0,     0,     0,    67,    54,    55,   161,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    57,     0,     0,     0,    58,    59,    60,    61,
      62,    63,    64,    65,     0,     0,    66,   100,     0,     0,
       0,     0,     0,     0,    67,   101,   102,   103,   104,   105,
     106,   100,   107,   108,   109,   110,   111,   112,   113,   101,
     102,   103,   104,   105,   106,     0,   107,   108,   109,   110,
     111,   112,   113,     0,   137,     0,     0,     0,     0,     0,
       0,   100,     0,     0,     0,     0,     0,     0,   177,   101,
     102,   103,   104,   105,   106,     0,   107,   108,   109,   110,
     111,   112,   113,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   186
  };

  const short
  LpcParser::yycheck_[] =
  {
      66,    60,    61,    42,    60,    61,   124,    13,    22,     0,
      24,     3,     4,   131,    19,    32,    26,    31,    57,    58,
      59,    31,   194,    62,    63,     5,    35,    57,     4,     4,
       5,   203,    37,    59,    26,    56,     2,    63,    68,    31,
     212,    33,    56,    57,    10,    66,    60,    47,    58,    24,
      26,    60,    32,   119,    68,    47,    62,    33,    61,    62,
     178,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    26,    56,    60,
      46,    27,   121,    61,    33,   124,    31,   216,    66,    35,
      31,   220,   131,    61,   266,   134,   268,   269,   137,   138,
      62,    62,   141,    64,   276,    67,    52,    53,   280,    62,
      56,   283,    59,   285,    67,    56,    57,   289,   290,    57,
      66,    56,    60,    31,    31,    59,    61,    68,    62,    59,
      68,    66,    27,   199,    59,    59,    61,    62,   177,   178,
      35,   180,   181,    59,    31,   204,    62,   186,   204,    59,
     216,    31,   218,    59,   220,    31,    62,    52,    53,    45,
      46,    47,    48,    49,   230,    29,    30,    32,    32,    59,
     229,    59,    62,   239,    62,    47,    48,    49,    62,    28,
      61,    62,    60,   249,    61,    62,   252,    36,    37,    38,
      39,    40,    41,    35,    43,    44,    45,    46,    47,    48,
      49,    61,    62,    31,   270,     3,   265,   273,     6,     7,
      63,     9,    10,    11,    12,    61,    14,    15,    16,    17,
      18,    19,    20,    28,    22,    23,    24,    61,    26,    61,
      62,    29,    30,    31,    32,    33,    41,    31,    43,    44,
      45,    46,    47,    48,    49,    61,    62,    31,    46,    31,
      61,    61,    50,    51,    52,    53,    54,    55,    56,    57,
      60,    59,    60,    35,    61,    63,    64,    62,     3,    61,
      68,     6,     7,    64,     9,    10,    11,    12,    61,    14,
      15,    16,    17,    18,    19,    20,    28,    22,    23,    24,
      60,    26,    60,    60,    29,    30,    31,    32,    33,    60,
      60,    43,    44,    45,    46,    47,    48,    49,    59,    59,
      65,    46,    60,     9,    21,    50,    51,    52,    53,    54,
      55,    56,    57,    59,    59,    60,    65,    22,    63,    24,
      60,    26,    60,    68,    29,    30,    31,    32,    33,    43,
      44,    45,    46,    47,    48,    49,    62,    59,    61,     8,
      59,    46,     2,     2,    36,    50,    51,    52,    53,    54,
      55,    56,    57,   207,    59,    60,     2,    22,    28,    24,
     272,   201,    83,    68,    29,    30,    31,    32,    38,    39,
      40,    41,    82,    43,    44,    45,    46,    47,    48,    49,
     273,    46,   181,   220,    52,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    52,    60,    -1,    28,    22,    64,
      24,    -1,    -1,    68,    -1,    29,    30,    31,    32,    40,
      41,    -1,    43,    44,    45,    46,    47,    48,    49,    -1,
      -1,    -1,    46,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    60,    -1,    22,    28,
      24,    -1,    -1,    67,    68,    29,    30,    31,    32,    -1,
      39,    40,    41,    -1,    43,    44,    45,    46,    47,    48,
      49,    -1,    46,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    59,    60,    -1,    22,    -1,
      24,    -1,    -1,    -1,    68,    29,    30,    31,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    60,    -1,    22,    -1,
      24,    -1,    -1,    -1,    68,    29,    30,    31,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    60,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    36,    37,    38,    39,    40,
      41,    28,    43,    44,    45,    46,    47,    48,    49,    36,
      37,    38,    39,    40,    41,    -1,    43,    44,    45,    46,
      47,    48,    49,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    65,    36,
      37,    38,    39,    40,    41,    -1,    43,    44,    45,    46,
      47,    48,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
  };

  const unsigned char
  LpcParser::yystos_[] =
  {
       0,    70,    71,     0,     3,     4,    26,    33,    47,    73,
      75,    76,    77,    78,    79,    83,    84,    32,    85,    85,
      72,    79,    80,    81,     4,    26,    33,    60,    31,     5,
      32,    82,    82,    59,    31,    59,    62,    85,    74,    31,
      59,    59,    35,    79,    81,    82,    26,    31,    58,    86,
      87,    88,    22,    24,    29,    30,    31,    46,    50,    51,
      52,    53,    54,    55,    56,    57,    60,    68,    85,    95,
      97,    98,    99,   119,   120,   121,   122,   123,    31,    59,
      79,    31,    61,    62,   120,   122,    97,    97,    97,    98,
      99,    98,    64,    97,   117,    67,    97,   118,    96,    97,
      28,    36,    37,    38,    39,    40,    41,    43,    44,    45,
      46,    47,    48,    49,    27,    35,    52,    53,    56,    66,
      57,    60,    68,    31,    60,    31,    59,    63,    89,    90,
      88,    63,   124,    61,    62,    64,    61,    65,    62,    67,
      61,    62,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    31,    96,
      31,    31,    97,   125,   126,   127,   128,    31,   128,    91,
     128,    97,    61,    97,    97,    61,    97,    65,    60,    67,
      35,    62,    61,    61,    92,    64,    65,    97,   128,    97,
     126,     6,     7,     9,    10,    11,    12,    14,    15,    16,
      17,    18,    19,    20,    23,    59,    64,    75,    77,    83,
      90,    93,    94,    96,   100,   101,   105,   106,   107,   108,
     109,   110,   111,   112,   114,   115,   116,    97,    61,    60,
      60,    60,    93,    60,    60,    59,    59,    59,    96,    60,
      95,    65,    93,    98,    80,    93,    59,    96,   100,   103,
      96,    96,   103,    99,    96,     9,    59,    96,    65,    21,
      59,    59,    59,    96,   104,    13,    61,    96,    61,    61,
      60,    61,    60,    59,    99,    93,    61,    93,    93,    96,
     113,    87,   104,    61,    93,     8,   102,    61,    93,    61,
      61,    93,    93,    59,    93,    93
  };

  const unsigned char
  LpcParser::yyr1_[] =
  {
       0,    69,    70,    71,    71,    71,    71,    72,    72,    74,
      73,    73,    75,    75,    76,    76,    77,    77,    78,    78,
      79,    79,    80,    80,    81,    81,    82,    82,    83,    84,
      84,    85,    85,    86,    86,    87,    87,    88,    88,    88,
      88,    89,    89,    91,    90,    92,    92,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    94,    94,    95,    95,    95,    96,
      96,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    98,    99,    99,
      99,    99,    99,   100,   100,   101,   102,   102,   103,   103,
     104,   104,   105,   106,   107,   108,   109,   110,   110,   111,
     113,   112,   114,   115,   116,   116,   117,   117,   118,   118,
     119,   119,   119,   119,   120,   120,   121,   121,   121,   122,
     122,   123,   123,   123,   123,   124,   124,   125,   126,   126,
     127,   127,   128,   128
  };

  const signed char
  LpcParser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     2,     3,     0,     2,     0,
       6,     3,     1,     1,     1,     2,     1,     2,     2,     3,
       0,     1,     1,     3,     2,     4,     0,     2,     4,     4,
       5,     1,     2,     0,     1,     1,     3,     3,     2,     1,
       2,     1,     1,     0,     4,     0,     2,     2,     2,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     2,     1,     1,     1,     1,
       3,     3,     3,     2,     2,     2,     2,     5,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     1,     1,     1,     1,     3,     1,
       1,     1,     4,     1,     3,     6,     0,     2,     1,     2,
       0,     1,     2,     7,     2,     6,     2,     4,     5,     7,
       0,     6,     7,     3,     2,     3,     1,     3,     3,     5,
       3,     4,     4,     3,     1,     3,     1,     1,     1,     3,
       2,     4,     3,     4,     6,     0,     3,     3,     1,     1,
       1,     3,     0,     1
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const LpcParser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "L_MOUNT", "L_INHERIT",
  "L_AS", "L_USING", "L_IF", "L_ELSE", "L_WHILE", "L_DO", "L_FOR",
  "L_FOREACH", "L_IN", "L_BREAK", "L_CONTINUE", "L_RETURN", "L_SWITCH",
  "L_CASE", "L_DEFAULT", "L_TRY", "L_CATCH", "L_NEW", "L_DESTRUCT",
  "L_SELF", "LOWER_THAN_ELSE", "L_TYPE", "L_ASSIGN", "L_ORDER",
  "L_INTEGER", "L_REAL_NUMBER", "L_IDENTIFIER", "L_STRING_LITERAL",
  "L_TYPE_MODIFIER", "POSITIONAL_IDENTIFIER", "'='", "'?'", "L_LOR",
  "L_LAND", "'|'", "'^'", "'&'", "'<'", "L_LSH", "L_RSH", "'+'", "'-'",
  "'*'", "'%'", "'/'", "L_NOT", "'~'", "L_INC", "L_DEC", "L_ARRAY_OPEN",
  "L_MAPPING_OPEN", "L_ARROW", "L_COLON_COLON", "L_ELLIPSIS", "';'", "'('",
  "')'", "','", "'{'", "'}'", "':'", "'['", "']'", "'.'", "$accept", "all",
  "program", "extra_semicolon", "def", "$@1", "storage_or_type",
  "typed_storage", "type_modifier_list", "function_head", "opt_star",
  "var_list", "new_var", "opt_as", "mount", "inheritance", "str_literal",
  "formal_parameters", "parameter_list", "new_param", "block_or_semicolon",
  "block", "$@2", "stmt_list", "stmt", "label", "const_expr", "comma_expr",
  "expr0", "lvalue", "expr4", "local_decl_list", "if_stmt",
  "opt_else_stmt", "loop_init", "opt_comma_expr", "for_begin", "for_stmt",
  "foreach_begin", "foreach_stmt", "while_begin", "while_stmt", "do_stmt",
  "switch_stmt", "$@3", "try_catch_stmt", "destruct_stmt", "return_stmt",
  "expr_list", "assoc_list", "aggregation", "defined_id", "call_prefix",
  "res_acquire", "function_call", "opt_init_list", "kwarg", "argument",
  "arguments", "arg_list", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  LpcParser::yyrline_[] =
  {
       0,   183,   183,   187,   188,   189,   195,   204,   205,   210,
     209,   248,   256,   261,   269,   271,   276,   277,   282,   291,
     300,   302,   307,   308,   327,   346,   372,   374,   379,   396,
     410,   427,   429,   434,   436,   440,   442,   455,   468,   476,
     488,   503,   505,   510,   509,   541,   543,   555,   557,   558,
     560,   561,   562,   563,   564,   565,   566,   567,   568,   569,
     570,   571,   572,   574,   580,   585,   593,   615,   620,   625,
     626,   631,   633,   635,   637,   639,   641,   643,   651,   653,
     655,   657,   659,   661,   663,   665,   667,   669,   671,   673,
     675,   677,   679,   681,   683,   684,   685,   690,   694,   696,
     698,   700,   701,   706,   708,   713,   724,   726,   731,   732,
     736,   738,   742,   750,   777,   785,   799,   807,   818,   845,
     856,   855,   868,   879,   888,   890,   895,   900,   909,   916,
     926,   928,   930,   932,   938,   946,   950,   951,   952,   956,
     958,   963,   971,   978,   985,   996,   998,  1003,  1011,  1012,
    1016,  1021,  1033,  1035
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
#line 5642 "lpc_parser.cpp"

#line 1038 "src/lpc_parser.yy"


// User subroutines section
void yy::LpcParser::error(const location_type& location, const std::string &msg) {
    throw parse_error(msg, location, yyget_extra(yyscanner));
}
