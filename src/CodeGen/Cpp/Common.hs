-- | Common code generation utility functions
module CodeGen.Cpp.Common where

import Prelude.Compat

import Data.Char
import Data.Monoid ((<>))
import Data.Text (Text)
import qualified Data.Text as T

import Language.PureScript.Crash
import Language.PureScript.Names

moduleNameToDir ::  ModuleName -> Text
moduleNameToDir (ModuleName pns) =
  T.intercalate "." (runProperName `map` pns)

moduleNameToCpp :: ModuleName -> Text
moduleNameToCpp (ModuleName pns) =
  let name = T.intercalate "_" (runProperName `map` pns)
  in if nameIsCppBuiltIn name then ("_" <> name <> "_") else name

-- | Convert an 'Ident' into a valid C++ identifier:
--
--  * Alphanumeric characters are kept unmodified.
--
--  * Reserved C++ identifiers are wrapped with '_'.
--
identToCpp :: Ident -> Text
identToCpp UnusedIdent = unusedName
identToCpp (Ident "$__unused") = unusedName
identToCpp (Ident name) = properToCpp name
identToCpp (GenIdent _ _) = internalError "GenIdent in identToCpp"

unusedName :: Text
unusedName = "_Unused_"

properToCpp :: Text -> Text
properToCpp name
  | nameIsCppReserved name || nameIsCppBuiltIn name || prefixIsReserved name = "_" <> name <> "_"
  | otherwise = T.concatMap identCharToText name

-- | Test if a string is a valid AST identifier without escaping.
identNeedsEscaping :: Text -> Bool
identNeedsEscaping s = s /= properToCpp s || T.null s

-- | Attempts to find a human-readable name for a symbol, if none has been specified returns the
-- ordinal value.
identCharToText :: Char -> Text
identCharToText c | isAlphaNum c = T.singleton c
identCharToText '_' = "_"
identCharToText '\'' = "_Prime_"
identCharToText '$' = "$"
identCharToText c = "_Code_Point_" <> T.pack (show (ord c))

-- | Checks whether an identifier name is reserved in C++.
nameIsCppReserved :: Text -> Bool
nameIsCppReserved name =
  name `elem` cppAnyReserved

-- | Checks whether a name matches a built-in value in C++.
nameIsCppBuiltIn :: Text -> Bool
nameIsCppBuiltIn name =
  name `elem`
    [ "final"
    , "override"
    , "posix"
    , "std"
    ]

cppAnyReserved :: [Text]
cppAnyReserved =
  concat
    [ cppKeywords
    , cppLiterals
    ]

cppKeywords :: [Text]
cppKeywords =
  [ "alignas"
  , "alignof"
  , "and"
  , "and_eq"
  , "asm"
  , "assert"
  , "atomic_cancel"
  , "atomic_commit"
  , "atomic_noexcept"
  , "auto"
  , "bitand"
  , "bitor"
  , "bool"
  , "break"
  , "case"
  , "catch"
  , "char"
  , "char16_t"
  , "char32_t"
  , "class"
  , "compl"
  , "concept"
  , "const"
  , "constexpr"
  , "const_cast"
  , "continue"
  , "concept"
  , "decltype"
  , "default"
  , "delete"
  , "do"
  , "double"
  , "dynamic_cast"
  , "else"
  , "enum"
  , "explicit"
  , "export"
  , "extern"
  , "false"
  , "float"
  , "for"
  , "friend"
  , "goto"
  , "if"
  , "import"
  , "inline"
  , "int"
  , "long"
  , "module"
  , "mutable"
  , "namespace"
  , "new"
  , "noexcept"
  , "not"
  , "not_eq"
  , "nullptr"
  , "nullptr_t"
  , "operator"
  , "or"
  , "or_eq"
  , "private"
  , "protected"
  , "public"
  , "register"
  , "reinterpret_cast"
  , "requires"
  , "return"
  , "short"
  , "signed"
  , "sizeof"
  , "static"
  , "static_assert"
  , "static_cast"
  , "struct"
  , "switch"
  , "synchronized"
  , "template"
  , "this"
  , "thread_local"
  , "throw"
  , "true"
  , "try"
  , "typedef"
  , "typeid"
  , "typename"
  , "typeof"
  , "union"
  , "unsigned"
  , "using"
  , "virtual"
  , "void"
  , "volatile"
  , "wchar_t"
  , "while"
  , "xor"
  , "xor_eq"
  ]

prefixIsReserved :: Text -> Bool
prefixIsReserved name =
  any (`T.isPrefixOf` name)
    [ "__"
    ]

cppLiterals :: [Text]
cppLiterals =
  [ "box"
  , "boxed"
  , "copy"
  , "errno"
  , "peek"
  , "purescript"
  , "runtime_error"
  , "setjmp"
  , "string"
  , "unbox"
  ]

dictType :: Text
dictType = "dict_t"

arrayType :: Text
arrayType = "array_t"

int :: Text
int = "int"

float :: Text
float = "double"

bool :: Text
bool = "bool"

string :: Text
string = "string"

unbox :: Text -> Text
unbox t = "unbox<" <> t <> ">"

arrayLengthFn :: Text
arrayLengthFn = "array_length"

unretainedSuffix :: Text
unretainedSuffix = "_Weak_"

tcoLoop :: Text
tcoLoop = "_TCO_Loop_"
