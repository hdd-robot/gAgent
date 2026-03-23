// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton interface for Bison LALR(1) parsers in C++

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


/**
 ** \file /home/hdd/lab/gAgent/build_test/src/messaging/fipa_acl_parser.hpp
 ** Define the gagent::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

#ifndef YY_YY_HOME_HDD_LAB_GAGENT_BUILD_TEST_SRC_MESSAGING_FIPA_ACL_PARSER_HPP_INCLUDED
# define YY_YY_HOME_HDD_LAB_GAGENT_BUILD_TEST_SRC_MESSAGING_FIPA_ACL_PARSER_HPP_INCLUDED
// "%code requires" blocks.
#line 11 "/home/hdd/lab/gAgent/src/messaging/fipa_acl.y"

#include <string>
#include <vector>
#include <gagent/messaging/ACLMessage.hpp>

namespace gagent {
    class FipaAclDriver;
}

#line 59 "/home/hdd/lab/gAgent/build_test/src/messaging/fipa_acl_parser.hpp"

# include <cassert>
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
# include "location.hh"
#include <typeinfo>
#ifndef YY_ASSERT
# include <cassert>
# define YY_ASSERT assert
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

#line 4 "/home/hdd/lab/gAgent/src/messaging/fipa_acl.y"
namespace gagent {
#line 200 "/home/hdd/lab/gAgent/build_test/src/messaging/fipa_acl_parser.hpp"




  /// A Bison parser.
  class FipaAclParser
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
      , yytypeid_ (YY_NULLPTR)
    {}

    /// Construct and fill.
    template <typename T>
    value_type (YY_RVREF (T) t)
      : yytypeid_ (&typeid (T))
    {
      YY_ASSERT (sizeof (T) <= size);
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
    {
      YY_ASSERT (!yytypeid_);
    }

# if 201103L <= YY_CPLUSPLUS
    /// Instantiate a \a T in here from \a t.
    template <typename T, typename... U>
    T&
    emplace (U&&... u)
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T (std::forward <U>(u)...);
    }
# else
    /// Instantiate an empty \a T in here.
    template <typename T>
    T&
    emplace ()
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T ();
    }

    /// Instantiate a \a T in here from \a t.
    template <typename T>
    T&
    emplace (const T& t)
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
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
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == typeid (T));
      YY_ASSERT (sizeof (T) <= size);
      return *yyas_<T> ();
    }

    /// Const accessor to a built \a T (for %printer).
    template <typename T>
    const T&
    as () const YY_NOEXCEPT
    {
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == typeid (T));
      YY_ASSERT (sizeof (T) <= size);
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
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == *that.yytypeid_);
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
      yytypeid_ = YY_NULLPTR;
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
      // performative
      char dummy1[sizeof (ACLMessage::Performative)];

      // agent_identifier
      char dummy2[sizeof (AgentIdentifier)];

      // "word"
      // "string"
      // expression
      // nested_exprs
      char dummy3[sizeof (std::string)];

      // receiver_spec
      // agent_list
      char dummy4[sizeof (std::vector<AgentIdentifier>)];
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

    /// Whether the content is built: if defined, the name of the stored type.
    const std::type_info *yytypeid_;
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
    END = 0,                       // "end of input"
    YYerror = 256,                 // error
    YYUNDEF = 257,                 // "invalid token"
    LPAREN = 258,                  // "("
    RPAREN = 259,                  // ")"
    KW_ACCEPT_PROPOSAL = 260,      // "accept-proposal"
    KW_AGREE = 261,                // "agree"
    KW_CANCEL = 262,               // "cancel"
    KW_CFP = 263,                  // "cfp"
    KW_CONFIRM = 264,              // "confirm"
    KW_DISCONFIRM = 265,           // "disconfirm"
    KW_FAILURE = 266,              // "failure"
    KW_INFORM = 267,               // "inform"
    KW_INFORM_IF = 268,            // "inform-if"
    KW_INFORM_REF = 269,           // "inform-ref"
    KW_NOT_UNDERSTOOD = 270,       // "not-understood"
    KW_PROPAGATE = 271,            // "propagate"
    KW_PROPOSE = 272,              // "propose"
    KW_PROXY = 273,                // "proxy"
    KW_QUERY_REF = 274,            // "query-ref"
    KW_REFUSE = 275,               // "refuse"
    KW_REJECT_PROPOSAL = 276,      // "reject-proposal"
    KW_REQUEST = 277,              // "request"
    KW_REQUEST_WHEN = 278,         // "request-when"
    KW_REQUEST_WHENEVER = 279,     // "request-whenever"
    KW_SUBSCRIBE = 280,            // "subscribe"
    KW_AGENT_IDENTIFIER = 281,     // "agent-identifier"
    KW_SET = 282,                  // "set"
    KW_SEQUENCE = 283,             // "sequence"
    PARAM_SENDER = 284,            // ":sender"
    PARAM_RECEIVER = 285,          // ":receiver"
    PARAM_CONTENT = 286,           // ":content"
    PARAM_REPLY_WITH = 287,        // ":reply-with"
    PARAM_IN_REPLY_TO = 288,       // ":in-reply-to"
    PARAM_REPLY_BY = 289,          // ":reply-by"
    PARAM_LANGUAGE = 290,          // ":language"
    PARAM_ENCODING = 291,          // ":encoding"
    PARAM_ONTOLOGY = 292,          // ":ontology"
    PARAM_PROTOCOL = 293,          // ":protocol"
    PARAM_CONVERSATION_ID = 294,   // ":conversation-id"
    PARAM_NAME = 295,              // ":name"
    PARAM_ADDRESSES = 296,         // ":addresses"
    PARAM_RESOLVERS = 297,         // ":resolvers"
    WORD = 298,                    // "word"
    STRING = 299                   // "string"
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
        YYNTOKENS = 45, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of input"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_LPAREN = 3,                            // "("
        S_RPAREN = 4,                            // ")"
        S_KW_ACCEPT_PROPOSAL = 5,                // "accept-proposal"
        S_KW_AGREE = 6,                          // "agree"
        S_KW_CANCEL = 7,                         // "cancel"
        S_KW_CFP = 8,                            // "cfp"
        S_KW_CONFIRM = 9,                        // "confirm"
        S_KW_DISCONFIRM = 10,                    // "disconfirm"
        S_KW_FAILURE = 11,                       // "failure"
        S_KW_INFORM = 12,                        // "inform"
        S_KW_INFORM_IF = 13,                     // "inform-if"
        S_KW_INFORM_REF = 14,                    // "inform-ref"
        S_KW_NOT_UNDERSTOOD = 15,                // "not-understood"
        S_KW_PROPAGATE = 16,                     // "propagate"
        S_KW_PROPOSE = 17,                       // "propose"
        S_KW_PROXY = 18,                         // "proxy"
        S_KW_QUERY_REF = 19,                     // "query-ref"
        S_KW_REFUSE = 20,                        // "refuse"
        S_KW_REJECT_PROPOSAL = 21,               // "reject-proposal"
        S_KW_REQUEST = 22,                       // "request"
        S_KW_REQUEST_WHEN = 23,                  // "request-when"
        S_KW_REQUEST_WHENEVER = 24,              // "request-whenever"
        S_KW_SUBSCRIBE = 25,                     // "subscribe"
        S_KW_AGENT_IDENTIFIER = 26,              // "agent-identifier"
        S_KW_SET = 27,                           // "set"
        S_KW_SEQUENCE = 28,                      // "sequence"
        S_PARAM_SENDER = 29,                     // ":sender"
        S_PARAM_RECEIVER = 30,                   // ":receiver"
        S_PARAM_CONTENT = 31,                    // ":content"
        S_PARAM_REPLY_WITH = 32,                 // ":reply-with"
        S_PARAM_IN_REPLY_TO = 33,                // ":in-reply-to"
        S_PARAM_REPLY_BY = 34,                   // ":reply-by"
        S_PARAM_LANGUAGE = 35,                   // ":language"
        S_PARAM_ENCODING = 36,                   // ":encoding"
        S_PARAM_ONTOLOGY = 37,                   // ":ontology"
        S_PARAM_PROTOCOL = 38,                   // ":protocol"
        S_PARAM_CONVERSATION_ID = 39,            // ":conversation-id"
        S_PARAM_NAME = 40,                       // ":name"
        S_PARAM_ADDRESSES = 41,                  // ":addresses"
        S_PARAM_RESOLVERS = 42,                  // ":resolvers"
        S_WORD = 43,                             // "word"
        S_STRING = 44,                           // "string"
        S_YYACCEPT = 45,                         // $accept
        S_start = 46,                            // start
        S_message = 47,                          // message
        S_performative = 48,                     // performative
        S_params = 49,                           // params
        S_param = 50,                            // param
        S_receiver_spec = 51,                    // receiver_spec
        S_agent_list = 52,                       // agent_list
        S_agent_identifier = 53,                 // agent_identifier
        S_agent_id_params = 54,                  // agent_id_params
        S_agent_id_param = 55,                   // agent_id_param
        S_url_list = 56,                         // url_list
        S_resolver_list = 57,                    // resolver_list
        S_expression = 58,                       // expression
        S_nested_exprs = 59                      // nested_exprs
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
      case symbol_kind::S_performative: // performative
        value.move< ACLMessage::Performative > (std::move (that.value));
        break;

      case symbol_kind::S_agent_identifier: // agent_identifier
        value.move< AgentIdentifier > (std::move (that.value));
        break;

      case symbol_kind::S_WORD: // "word"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_nested_exprs: // nested_exprs
        value.move< std::string > (std::move (that.value));
        break;

      case symbol_kind::S_receiver_spec: // receiver_spec
      case symbol_kind::S_agent_list: // agent_list
        value.move< std::vector<AgentIdentifier> > (std::move (that.value));
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
      basic_symbol (typename Base::kind_type t, ACLMessage::Performative&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ACLMessage::Performative& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, AgentIdentifier&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const AgentIdentifier& v, const location_type& l)
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

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<AgentIdentifier>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<AgentIdentifier>& v, const location_type& l)
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
      case symbol_kind::S_performative: // performative
        value.template destroy< ACLMessage::Performative > ();
        break;

      case symbol_kind::S_agent_identifier: // agent_identifier
        value.template destroy< AgentIdentifier > ();
        break;

      case symbol_kind::S_WORD: // "word"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_nested_exprs: // nested_exprs
        value.template destroy< std::string > ();
        break;

      case symbol_kind::S_receiver_spec: // receiver_spec
      case symbol_kind::S_agent_list: // agent_list
        value.template destroy< std::vector<AgentIdentifier> > ();
        break;

      default:
        break;
    }

        Base::clear ();
      }

      /// The user-facing name of this symbol.
      std::string name () const YY_NOEXCEPT
      {
        return FipaAclParser::symbol_name (this->kind ());
      }

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
      {
#if !defined _MSC_VER || defined __clang__
        YY_ASSERT (tok == token::END
                   || (token::YYerror <= tok && tok <= token::PARAM_RESOLVERS));
#endif
      }
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, std::string v, location_type l)
        : super_type (token_kind_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const std::string& v, const location_type& l)
        : super_type (token_kind_type (tok), v, l)
#endif
      {
#if !defined _MSC_VER || defined __clang__
        YY_ASSERT ((token::WORD <= tok && tok <= token::STRING));
#endif
      }
    };

    /// Build a parser object.
    FipaAclParser (FipaAclDriver& driver_yyarg);
    virtual ~FipaAclParser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    FipaAclParser (const FipaAclParser&) = delete;
    /// Non copyable.
    FipaAclParser& operator= (const FipaAclParser&) = delete;
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

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static std::string symbol_name (symbol_kind_type yysymbol);

    // Implementation of make_symbol for each token kind.
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_END (location_type l)
      {
        return symbol_type (token::END, std::move (l));
      }
#else
      static
      symbol_type
      make_END (const location_type& l)
      {
        return symbol_type (token::END, l);
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
      make_LPAREN (location_type l)
      {
        return symbol_type (token::LPAREN, std::move (l));
      }
#else
      static
      symbol_type
      make_LPAREN (const location_type& l)
      {
        return symbol_type (token::LPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RPAREN (location_type l)
      {
        return symbol_type (token::RPAREN, std::move (l));
      }
#else
      static
      symbol_type
      make_RPAREN (const location_type& l)
      {
        return symbol_type (token::RPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_ACCEPT_PROPOSAL (location_type l)
      {
        return symbol_type (token::KW_ACCEPT_PROPOSAL, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_ACCEPT_PROPOSAL (const location_type& l)
      {
        return symbol_type (token::KW_ACCEPT_PROPOSAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_AGREE (location_type l)
      {
        return symbol_type (token::KW_AGREE, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_AGREE (const location_type& l)
      {
        return symbol_type (token::KW_AGREE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_CANCEL (location_type l)
      {
        return symbol_type (token::KW_CANCEL, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_CANCEL (const location_type& l)
      {
        return symbol_type (token::KW_CANCEL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_CFP (location_type l)
      {
        return symbol_type (token::KW_CFP, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_CFP (const location_type& l)
      {
        return symbol_type (token::KW_CFP, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_CONFIRM (location_type l)
      {
        return symbol_type (token::KW_CONFIRM, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_CONFIRM (const location_type& l)
      {
        return symbol_type (token::KW_CONFIRM, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_DISCONFIRM (location_type l)
      {
        return symbol_type (token::KW_DISCONFIRM, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_DISCONFIRM (const location_type& l)
      {
        return symbol_type (token::KW_DISCONFIRM, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_FAILURE (location_type l)
      {
        return symbol_type (token::KW_FAILURE, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_FAILURE (const location_type& l)
      {
        return symbol_type (token::KW_FAILURE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_INFORM (location_type l)
      {
        return symbol_type (token::KW_INFORM, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_INFORM (const location_type& l)
      {
        return symbol_type (token::KW_INFORM, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_INFORM_IF (location_type l)
      {
        return symbol_type (token::KW_INFORM_IF, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_INFORM_IF (const location_type& l)
      {
        return symbol_type (token::KW_INFORM_IF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_INFORM_REF (location_type l)
      {
        return symbol_type (token::KW_INFORM_REF, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_INFORM_REF (const location_type& l)
      {
        return symbol_type (token::KW_INFORM_REF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_NOT_UNDERSTOOD (location_type l)
      {
        return symbol_type (token::KW_NOT_UNDERSTOOD, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_NOT_UNDERSTOOD (const location_type& l)
      {
        return symbol_type (token::KW_NOT_UNDERSTOOD, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_PROPAGATE (location_type l)
      {
        return symbol_type (token::KW_PROPAGATE, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_PROPAGATE (const location_type& l)
      {
        return symbol_type (token::KW_PROPAGATE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_PROPOSE (location_type l)
      {
        return symbol_type (token::KW_PROPOSE, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_PROPOSE (const location_type& l)
      {
        return symbol_type (token::KW_PROPOSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_PROXY (location_type l)
      {
        return symbol_type (token::KW_PROXY, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_PROXY (const location_type& l)
      {
        return symbol_type (token::KW_PROXY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_QUERY_REF (location_type l)
      {
        return symbol_type (token::KW_QUERY_REF, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_QUERY_REF (const location_type& l)
      {
        return symbol_type (token::KW_QUERY_REF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_REFUSE (location_type l)
      {
        return symbol_type (token::KW_REFUSE, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_REFUSE (const location_type& l)
      {
        return symbol_type (token::KW_REFUSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_REJECT_PROPOSAL (location_type l)
      {
        return symbol_type (token::KW_REJECT_PROPOSAL, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_REJECT_PROPOSAL (const location_type& l)
      {
        return symbol_type (token::KW_REJECT_PROPOSAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_REQUEST (location_type l)
      {
        return symbol_type (token::KW_REQUEST, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_REQUEST (const location_type& l)
      {
        return symbol_type (token::KW_REQUEST, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_REQUEST_WHEN (location_type l)
      {
        return symbol_type (token::KW_REQUEST_WHEN, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_REQUEST_WHEN (const location_type& l)
      {
        return symbol_type (token::KW_REQUEST_WHEN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_REQUEST_WHENEVER (location_type l)
      {
        return symbol_type (token::KW_REQUEST_WHENEVER, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_REQUEST_WHENEVER (const location_type& l)
      {
        return symbol_type (token::KW_REQUEST_WHENEVER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_SUBSCRIBE (location_type l)
      {
        return symbol_type (token::KW_SUBSCRIBE, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_SUBSCRIBE (const location_type& l)
      {
        return symbol_type (token::KW_SUBSCRIBE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_AGENT_IDENTIFIER (location_type l)
      {
        return symbol_type (token::KW_AGENT_IDENTIFIER, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_AGENT_IDENTIFIER (const location_type& l)
      {
        return symbol_type (token::KW_AGENT_IDENTIFIER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_SET (location_type l)
      {
        return symbol_type (token::KW_SET, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_SET (const location_type& l)
      {
        return symbol_type (token::KW_SET, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_KW_SEQUENCE (location_type l)
      {
        return symbol_type (token::KW_SEQUENCE, std::move (l));
      }
#else
      static
      symbol_type
      make_KW_SEQUENCE (const location_type& l)
      {
        return symbol_type (token::KW_SEQUENCE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_SENDER (location_type l)
      {
        return symbol_type (token::PARAM_SENDER, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_SENDER (const location_type& l)
      {
        return symbol_type (token::PARAM_SENDER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_RECEIVER (location_type l)
      {
        return symbol_type (token::PARAM_RECEIVER, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_RECEIVER (const location_type& l)
      {
        return symbol_type (token::PARAM_RECEIVER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_CONTENT (location_type l)
      {
        return symbol_type (token::PARAM_CONTENT, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_CONTENT (const location_type& l)
      {
        return symbol_type (token::PARAM_CONTENT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_REPLY_WITH (location_type l)
      {
        return symbol_type (token::PARAM_REPLY_WITH, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_REPLY_WITH (const location_type& l)
      {
        return symbol_type (token::PARAM_REPLY_WITH, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_IN_REPLY_TO (location_type l)
      {
        return symbol_type (token::PARAM_IN_REPLY_TO, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_IN_REPLY_TO (const location_type& l)
      {
        return symbol_type (token::PARAM_IN_REPLY_TO, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_REPLY_BY (location_type l)
      {
        return symbol_type (token::PARAM_REPLY_BY, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_REPLY_BY (const location_type& l)
      {
        return symbol_type (token::PARAM_REPLY_BY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_LANGUAGE (location_type l)
      {
        return symbol_type (token::PARAM_LANGUAGE, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_LANGUAGE (const location_type& l)
      {
        return symbol_type (token::PARAM_LANGUAGE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_ENCODING (location_type l)
      {
        return symbol_type (token::PARAM_ENCODING, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_ENCODING (const location_type& l)
      {
        return symbol_type (token::PARAM_ENCODING, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_ONTOLOGY (location_type l)
      {
        return symbol_type (token::PARAM_ONTOLOGY, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_ONTOLOGY (const location_type& l)
      {
        return symbol_type (token::PARAM_ONTOLOGY, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_PROTOCOL (location_type l)
      {
        return symbol_type (token::PARAM_PROTOCOL, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_PROTOCOL (const location_type& l)
      {
        return symbol_type (token::PARAM_PROTOCOL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_CONVERSATION_ID (location_type l)
      {
        return symbol_type (token::PARAM_CONVERSATION_ID, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_CONVERSATION_ID (const location_type& l)
      {
        return symbol_type (token::PARAM_CONVERSATION_ID, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_NAME (location_type l)
      {
        return symbol_type (token::PARAM_NAME, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_NAME (const location_type& l)
      {
        return symbol_type (token::PARAM_NAME, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_ADDRESSES (location_type l)
      {
        return symbol_type (token::PARAM_ADDRESSES, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_ADDRESSES (const location_type& l)
      {
        return symbol_type (token::PARAM_ADDRESSES, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAM_RESOLVERS (location_type l)
      {
        return symbol_type (token::PARAM_RESOLVERS, std::move (l));
      }
#else
      static
      symbol_type
      make_PARAM_RESOLVERS (const location_type& l)
      {
        return symbol_type (token::PARAM_RESOLVERS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_WORD (std::string v, location_type l)
      {
        return symbol_type (token::WORD, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_WORD (const std::string& v, const location_type& l)
      {
        return symbol_type (token::WORD, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STRING (std::string v, location_type l)
      {
        return symbol_type (token::STRING, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_STRING (const std::string& v, const location_type& l)
      {
        return symbol_type (token::STRING, v, l);
      }
#endif


    class context
    {
    public:
      context (const FipaAclParser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const YY_NOEXCEPT { return yyla_; }
      symbol_kind_type token () const YY_NOEXCEPT { return yyla_.kind (); }
      const location_type& location () const YY_NOEXCEPT { return yyla_.location; }

      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const FipaAclParser& yyparser_;
      const symbol_type& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    FipaAclParser (const FipaAclParser&);
    /// Non copyable.
    FipaAclParser& operator= (const FipaAclParser&);
#endif


    /// Stored state numbers (used for stacks).
    typedef signed char state_type;

    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;
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

    static const signed char yypact_ninf_;
    static const signed char yytable_ninf_;

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_kind_type enum.
    static symbol_kind_type yytranslate_ (int t) YY_NOEXCEPT;

    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *yystr);

    /// For a symbol, its name in clear.
    static const char* const yytname_[];


    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const signed char yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const signed char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const signed char yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const signed char yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const signed char yytable_[];

    static const signed char yycheck_[];

    // YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
    // state STATE-NUM.
    static const signed char yystos_[];

    // YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.
    static const signed char yyr1_[];

    // YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.
    static const signed char yyr2_[];


#if YYDEBUG
    // YYRLINE[YYN] -- Source line where rule number YYN was defined.
    static const unsigned char yyrline_[];
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
      yylast_ = 81,     ///< Last index in yytable_.
      yynnts_ = 15,  ///< Number of nonterminal symbols.
      yyfinal_ = 26 ///< Termination state number.
    };


    // User arguments.
    FipaAclDriver& driver;

  };

  inline
  FipaAclParser::symbol_kind_type
  FipaAclParser::yytranslate_ (int t) YY_NOEXCEPT
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44
    };
    // Last valid token kind.
    const int code_max = 299;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

  // basic_symbol.
  template <typename Base>
  FipaAclParser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value ()
    , location (that.location)
  {
    switch (this->kind ())
    {
      case symbol_kind::S_performative: // performative
        value.copy< ACLMessage::Performative > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_agent_identifier: // agent_identifier
        value.copy< AgentIdentifier > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_WORD: // "word"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_nested_exprs: // nested_exprs
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_receiver_spec: // receiver_spec
      case symbol_kind::S_agent_list: // agent_list
        value.copy< std::vector<AgentIdentifier> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }




  template <typename Base>
  FipaAclParser::symbol_kind_type
  FipaAclParser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  FipaAclParser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  FipaAclParser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case symbol_kind::S_performative: // performative
        value.move< ACLMessage::Performative > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_agent_identifier: // agent_identifier
        value.move< AgentIdentifier > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_WORD: // "word"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_nested_exprs: // nested_exprs
        value.move< std::string > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_receiver_spec: // receiver_spec
      case symbol_kind::S_agent_list: // agent_list
        value.move< std::vector<AgentIdentifier> > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
  }

  // by_kind.
  inline
  FipaAclParser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  inline
  FipaAclParser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  inline
  FipaAclParser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  inline
  FipaAclParser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  inline
  void
  FipaAclParser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  inline
  void
  FipaAclParser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  inline
  FipaAclParser::symbol_kind_type
  FipaAclParser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  inline
  FipaAclParser::symbol_kind_type
  FipaAclParser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


#line 4 "/home/hdd/lab/gAgent/src/messaging/fipa_acl.y"
} // gagent
#line 2161 "/home/hdd/lab/gAgent/build_test/src/messaging/fipa_acl_parser.hpp"




#endif // !YY_YY_HOME_HDD_LAB_GAGENT_BUILD_TEST_SRC_MESSAGING_FIPA_ACL_PARSER_HPP_INCLUDED
