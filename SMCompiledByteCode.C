///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMFunction.h"
#include "SMCompiledByteCode.h"

namespace SymbolicMath
{

template <typename T>
CompiledByteCode<T>::CompiledByteCode(Function<T> & fb) : Transform<T>(fb)
{
  // determine required stack size
  auto current_max = std::make_pair(0, 0);
  fb.root().stackDepth(current_max);
  _stack.resize(current_max.second);

  apply();
}

template <typename T>
void
CompiledByteCode<T>::operator()(SymbolData<T> * n)
{
  fatalError("Symbol in compiled function");
}

template <typename T>
void
CompiledByteCode<T>::operator()(UnaryOperatorData<T> * n)
{
  static const std::map<UnaryOperatorType, VMInstruction> map = {
      {UnaryOperatorType::PLUS, VMInstruction::UO_PLUS},
      {UnaryOperatorType::MINUS, VMInstruction::UO_MINUS},
      {UnaryOperatorType::FACULTY, VMInstruction::UO_FACULTY},
      {UnaryOperatorType::NOT, VMInstruction::UO_NOT}};

  n->_args[0].apply(*this);

  auto vi = map.find(n->_type);
  if (vi == map.end())
    fatalError("Invalid instruction");
  _byte_code.emplace_back(static_cast<int>(vi->second));
}

template <typename T>
void
CompiledByteCode<T>::operator()(BinaryOperatorData<T> * n)
{
  static const std::map<BinaryOperatorType, VMInstruction> map = {
      {BinaryOperatorType::SUBTRACTION, VMInstruction::BO_SUBTRACTION},
      {BinaryOperatorType::DIVISION, VMInstruction::BO_DIVISION},
      {BinaryOperatorType::MODULO, VMInstruction::BO_MODULO},
      {BinaryOperatorType::POWER, VMInstruction::BO_POWER},
      {BinaryOperatorType::LOGICAL_OR, VMInstruction::BO_LOGICAL_OR},
      {BinaryOperatorType::LOGICAL_AND, VMInstruction::BO_LOGICAL_AND},
      {BinaryOperatorType::LESS_THAN, VMInstruction::BO_LESS_THAN},
      {BinaryOperatorType::GREATER_THAN, VMInstruction::BO_GREATER_THAN},
      {BinaryOperatorType::LESS_EQUAL, VMInstruction::BO_LESS_EQUAL},
      {BinaryOperatorType::GREATER_EQUAL, VMInstruction::BO_GREATER_EQUAL},
      {BinaryOperatorType::EQUAL, VMInstruction::BO_EQUAL},
      {BinaryOperatorType::NOT_EQUAL, VMInstruction::BO_NOT_EQUAL},
      {BinaryOperatorType::ASSIGNMENT, VMInstruction::BO_ASSIGNMENT},
      {BinaryOperatorType::LIST, VMInstruction::BO_LIST}};

  n->_args[0].apply(*this);
  n->_args[1].apply(*this);

  auto vi = map.find(n->_type);
  if (vi == map.end())
    fatalError("Invalid instruction");
  _byte_code.emplace_back(static_cast<int>(vi->second));
}

template <typename T>
void
CompiledByteCode<T>::operator()(MultinaryOperatorData<T> * n)
{
  static const std::map<MultinaryOperatorType, VMInstruction> map = {
      {MultinaryOperatorType::ADDITION, VMInstruction::MO_ADDITION},
      {MultinaryOperatorType::MULTIPLICATION, VMInstruction::MO_MULTIPLICATION},
      {MultinaryOperatorType::COMPONENT, VMInstruction::MO_COMPONENT},
      {MultinaryOperatorType::LIST, VMInstruction::MO_LIST}};

  const int nargs = static_cast<int>(n->_args.size());
  for (auto arg : n->_args)
    arg.apply(*this);

  if (nargs < 2)
    return;

  auto vi = map.find(n->_type);
  if (vi == map.end())
    fatalError("Invalid instruction");
  _byte_code.emplace_back(static_cast<int>(vi->second));
  _byte_code.emplace_back(nargs - 1);
}

template <typename T>
void
CompiledByteCode<T>::operator()(UnaryFunctionData<T> * n)
{
  static const std::map<UnaryFunctionType, VMInstruction> map = {
      {UnaryFunctionType::ABS, VMInstruction::UF_ABS},
      {UnaryFunctionType::ACOS, VMInstruction::UF_ACOS},
      {UnaryFunctionType::ACOSH, VMInstruction::UF_ACOSH},
      {UnaryFunctionType::ARG, VMInstruction::UF_ARG},
      {UnaryFunctionType::ASIN, VMInstruction::UF_ASIN},
      {UnaryFunctionType::ASINH, VMInstruction::UF_ASINH},
      {UnaryFunctionType::ATAN, VMInstruction::UF_ATAN},
      {UnaryFunctionType::ATANH, VMInstruction::UF_ATANH},
      {UnaryFunctionType::CBRT, VMInstruction::UF_CBRT},
      {UnaryFunctionType::CEIL, VMInstruction::UF_CEIL},
      {UnaryFunctionType::CONJ, VMInstruction::UF_CONJ},
      {UnaryFunctionType::COS, VMInstruction::UF_COS},
      {UnaryFunctionType::COSH, VMInstruction::UF_COSH},
      {UnaryFunctionType::COT, VMInstruction::UF_COT},
      {UnaryFunctionType::CSC, VMInstruction::UF_CSC},
      {UnaryFunctionType::ERF, VMInstruction::UF_ERF},
      {UnaryFunctionType::ERFC, VMInstruction::UF_ERFC},
      {UnaryFunctionType::EXP, VMInstruction::UF_EXP},
      {UnaryFunctionType::EXP2, VMInstruction::UF_EXP2},
      {UnaryFunctionType::FLOOR, VMInstruction::UF_FLOOR},
      {UnaryFunctionType::IMAG, VMInstruction::UF_IMAG},
      {UnaryFunctionType::INT, VMInstruction::UF_INT},
      {UnaryFunctionType::LOG, VMInstruction::UF_LOG},
      {UnaryFunctionType::LOG10, VMInstruction::UF_LOG10},
      {UnaryFunctionType::LOG2, VMInstruction::UF_LOG2},
      {UnaryFunctionType::REAL, VMInstruction::UF_REAL},
      {UnaryFunctionType::SEC, VMInstruction::UF_SEC},
      {UnaryFunctionType::SIN, VMInstruction::UF_SIN},
      {UnaryFunctionType::SINH, VMInstruction::UF_SINH},
      {UnaryFunctionType::SQRT, VMInstruction::UF_SQRT},
      {UnaryFunctionType::T, VMInstruction::UF_T},
      {UnaryFunctionType::TAN, VMInstruction::UF_TAN},
      {UnaryFunctionType::TANH, VMInstruction::UF_TANH},
      {UnaryFunctionType::TRUNC, VMInstruction::UF_TRUNC}};

  n->_args[0].apply(*this);

  auto vi = map.find(n->_type);
  if (vi == map.end())
    fatalError("Invalid instruction");
  _byte_code.emplace_back(static_cast<int>(vi->second));
}

template <typename T>
void
CompiledByteCode<T>::operator()(BinaryFunctionData<T> * n)
{
  static const std::map<BinaryFunctionType, VMInstruction> map = {
      {BinaryFunctionType::ATAN2, VMInstruction::BF_ATAN2},
      {BinaryFunctionType::HYPOT, VMInstruction::BF_HYPOT},
      {BinaryFunctionType::MAX, VMInstruction::BF_MAX},
      {BinaryFunctionType::MIN, VMInstruction::BF_MIN},
      {BinaryFunctionType::PLOG, VMInstruction::BF_PLOG},
      {BinaryFunctionType::POLAR, VMInstruction::BF_POLAR},
      {BinaryFunctionType::POW, VMInstruction::BF_POW}};

  n->_args[0].apply(*this);
  n->_args[1].apply(*this);

  auto vi = map.find(n->_type);
  if (vi == map.end())
    fatalError("Invalid instruction");
  _byte_code.emplace_back(static_cast<int>(vi->second));
}

template <typename T>
void
CompiledByteCode<T>::operator()(RealNumberData<T> * n)
{
  _byte_code.emplace_back(static_cast<int>(VMInstruction::LOAD_IMMEDIATE_REAL));

  // find pointer in _immed, or add if not found
  for (int i = 0; i < _immed.size(); ++i)
    if (_immed[i] == n->_value)
    {
      _byte_code.emplace_back(i);
      return;
    }
  _immed.emplace_back(n->_value);
  _byte_code.emplace_back(_immed.size() - 1);
}

template <typename T>
void
CompiledByteCode<T>::operator()(RealReferenceData<T> * n)
{
  _byte_code.emplace_back(static_cast<int>(VMInstruction::LOAD_VARIABLE_REAL));

  // find pointer in _vars, or add if not found
  for (int i = 0; i < _vars.size(); ++i)
    if (_vars[i] == &n->_ref)
    {
      _byte_code.emplace_back(i);
      return;
    }
  _vars.emplace_back(&n->_ref);
  _byte_code.emplace_back(_vars.size() - 1);
}

template <typename T>
void
CompiledByteCode<T>::operator()(RealArrayReferenceData<T> * n)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledByteCode<T>::operator()(LocalVariableData<T> * n)
{
  fatalError("Not implemented");
}

template <typename T>
void
CompiledByteCode<T>::operator()(ConditionalData<T> * n)
{
  n->_args[0].apply(*this);
  _byte_code.emplace_back(static_cast<int>(VMInstruction::CONDITIONAL));
  // jump label placeholder
  const auto conditional_ip = _byte_code.size();
  _byte_code.emplace_back(0);
  // true branch
  n->_args[1].apply(*this);
  // jump past false at the end of the true branch
  _byte_code.emplace_back(static_cast<int>(VMInstruction::JUMP));
  // jump label placeholder
  const auto jump_past_false_ip = _byte_code.size();
  _byte_code.emplace_back(0);
  // set jump to false ip on conditional instruction
  _byte_code[conditional_ip] = _byte_code.size();
  // false branch
  n->_args[2].apply(*this);
  // set jump past false target
  _byte_code[jump_past_false_ip] = _byte_code.size();
}

template <typename T>
void
CompiledByteCode<T>::operator()(IntegerPowerData<T> * n)
{
  n->_arg.apply(*this);
  _byte_code.emplace_back(static_cast<int>(VMInstruction::INTEGER_POWER));
  _byte_code.emplace_back(n->_exponent);
}

template <typename T>
T
CompiledByteCode<T>::operator()()
{
  // initialize instruction and stack pointer and loop over byte code
  const auto byte_code_size = _byte_code.size();
  int ip = 0, sp = -1;
  do
  {
#ifdef DEBUG
    std::cout << "mon: " << ip << ' ' << _byte_code[ip] << ' ' << sp << '\n';
#endif

    switch (static_cast<VMInstruction>(_byte_code[ip]))
    {
      case VMInstruction::LOAD_IMMEDIATE_REAL:
        _stack[++sp] = _immed[_byte_code[++ip]];
        break;

      case VMInstruction::LOAD_VARIABLE_REAL:
        _stack[++sp] = *_vars[_byte_code[++ip]];
        break;

      case VMInstruction::MO_ADDITION:
      {
        // take one summand off the stack and loop over remaining summands
        const auto & num = _byte_code[++ip];
        auto sum = _stack[sp--];
        const int end = sp - num;
        for (int i = sp; i > end; --i)
          sum += _stack[i];
        sp -= num;

        // put sum on stack
        _stack[++sp] = sum;
        break;
      }

      case VMInstruction::MO_MULTIPLICATION:
      {
        // take one factor off the stack and loop over remaining factors
        const auto & num = _byte_code[++ip];
        auto prod = _stack[sp--];
        const int end = sp - num;
        for (int i = sp; i > end; --i)
          prod *= _stack[i];
        sp -= num;

        // put product on stack
        _stack[++sp] = prod;
        break;
      }

      case VMInstruction::UO_MINUS:
        _stack[sp] = -_stack[sp];
        break;

      case VMInstruction::BO_SUBTRACTION:
        --sp;
        _stack[sp] = _stack[sp] - _stack[sp + 1];
        break;

      case VMInstruction::BO_DIVISION:
        --sp;
        _stack[sp] = _stack[sp] / _stack[sp + 1];
        break;

      case VMInstruction::BO_MODULO:
        --sp;
        _stack[sp] = std::fmod(_stack[sp], _stack[sp + 1]);
        break;

      case VMInstruction::BO_POWER:
        --sp;
        _stack[sp] = std::pow(_stack[sp], _stack[sp + 1]);
        break;

      case VMInstruction::BO_LOGICAL_OR:
        --sp;
        _stack[sp] = _stack[sp] || _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::BO_LOGICAL_AND:
        --sp;
        _stack[sp] = _stack[sp] && _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::BO_LESS_THAN:
        --sp;
        _stack[sp] = _stack[sp] < _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::BO_GREATER_THAN:
        --sp;
        _stack[sp] = _stack[sp] > _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::BO_LESS_EQUAL:
        --sp;
        _stack[sp] = _stack[sp] <= _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::BO_GREATER_EQUAL:
        --sp;
        _stack[sp] = _stack[sp] >= _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::BO_EQUAL:
        --sp;
        _stack[sp] = _stack[sp] == _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::BO_NOT_EQUAL:
        --sp;
        _stack[sp] = _stack[sp] != _stack[sp + 1] ? 1.0 : 0.0;
        break;

      case VMInstruction::UF_ABS:
        _stack[sp] = std::abs(_stack[sp]);
        break;

      case VMInstruction::UF_ACOS:
        _stack[sp] = std::acos(_stack[sp]);
        break;

      case VMInstruction::UF_ACOSH:
        _stack[sp] = std::acosh(_stack[sp]);
        break;

      case VMInstruction::UF_ASIN:
        _stack[sp] = std::asin(_stack[sp]);
        break;

      case VMInstruction::UF_ASINH:
        _stack[sp] = std::asinh(_stack[sp]);
        break;

      case VMInstruction::UF_ATAN:
        _stack[sp] = std::atan(_stack[sp]);
        break;

      case VMInstruction::UF_ATANH:
        _stack[sp] = std::atanh(_stack[sp]);
        break;

      case VMInstruction::UF_CBRT:
        _stack[sp] = std::cbrt(_stack[sp]);
        break;

      case VMInstruction::UF_CEIL:
        _stack[sp] = std::ceil(_stack[sp]);
        break;

      case VMInstruction::UF_COS:
        _stack[sp] = std::cos(_stack[sp]);
        break;

      case VMInstruction::UF_COSH:
        _stack[sp] = std::cosh(_stack[sp]);
        break;

      case VMInstruction::UF_COT:
        _stack[sp] = 1.0 / std::tan(_stack[sp]);
        break;

      case VMInstruction::UF_CSC:
        _stack[sp] = 1.0 / std::sin(_stack[sp]);
        break;

      case VMInstruction::UF_ERF:
        _stack[sp] = std::erf(_stack[sp]);
        break;

      case VMInstruction::UF_ERFC:
        _stack[sp] = std::erfc(_stack[sp]);
        break;

      case VMInstruction::UF_EXP:
        _stack[sp] = std::exp(_stack[sp]);
        break;

      case VMInstruction::UF_EXP2:
        _stack[sp] = std::exp2(_stack[sp]);
        break;

      case VMInstruction::UF_FLOOR:
        _stack[sp] = std::floor(_stack[sp]);
        break;

      case VMInstruction::UF_INT:
        _stack[sp] = std::round(_stack[sp]);
        break;

      case VMInstruction::UF_LOG:
        _stack[sp] = std::log(_stack[sp]);
        break;

      case VMInstruction::UF_LOG10:
        _stack[sp] = std::log10(_stack[sp]);
        break;

      case VMInstruction::UF_LOG2:
        _stack[sp] = std::log2(_stack[sp]);
        break;

      case VMInstruction::UF_SEC:
        _stack[sp] = 1.0 / std::cos(_stack[sp]);
        break;

      case VMInstruction::UF_SIN:
        _stack[sp] = std::sin(_stack[sp]);
        break;

      case VMInstruction::UF_SINH:
        _stack[sp] = std::sinh(_stack[sp]);
        break;

      case VMInstruction::UF_SQRT:
        _stack[sp] = std::sqrt(_stack[sp]);
        break;

      case VMInstruction::UF_TAN:
        _stack[sp] = std::tan(_stack[sp]);
        break;

      case VMInstruction::UF_TANH:
        _stack[sp] = std::tanh(_stack[sp]);
        break;

      case VMInstruction::UF_TRUNC:
        _stack[sp] = static_cast<int>(_stack[sp]);
        break;

      case VMInstruction::BF_ATAN2:
        --sp;
        _stack[sp] = std::atan2(_stack[sp], _stack[sp + 1]);
        break;

      case VMInstruction::BF_HYPOT:
        --sp;
        _stack[sp] = std::sqrt(_stack[sp] * _stack[sp] + _stack[sp + 1] * _stack[sp + 1]);
        break;

      case VMInstruction::BF_MAX:
        --sp;
        _stack[sp] = std::max(_stack[sp], _stack[sp + 1]);
        break;

      case VMInstruction::BF_MIN:
        --sp;
        _stack[sp] = std::min(_stack[sp], _stack[sp + 1]);
        break;

      case VMInstruction::BF_PLOG:
      {
        --sp;
        const auto & a = _stack[sp];
        const auto & b = _stack[sp + 1];
        _stack[sp] = a < b ? std::log(b) + (a - b) / b - (a - b) * (a - b) / (2.0 * b * b) +
                                 (a - b) * (a - b) * (a - b) / (3.0 * b * b * b)
                           : std::log(a);
        break;
      }

      case VMInstruction::BF_POW:
        --sp;
        _stack[sp] = std::pow(_stack[sp], _stack[sp + 1]);
        break;

      case VMInstruction::JUMP:
        ip = _byte_code[++ip] - 1;
        break;

      case VMInstruction::CONDITIONAL:
        ++ip;
        if (_stack[sp--] == 0)
          ip = _byte_code[ip] - 1;
        break;

      case VMInstruction::INTEGER_POWER:
      {
        auto x = _stack[sp];
        _stack[sp] = 1.0;
        int e = std::abs(_byte_code[++ip]);

        while (true)
        {
          // if bit 0 is set multiply the current power of two factor of the exponent
          if (e & 1)
            _stack[sp] *= x;

          // x is incrementally set to consecutive powers of powers of two
          x *= x;

          if (e == 0)
            break;

          // bit shift the exponent down
          e >>= 1;
        }

        if (_byte_code[ip] < 0)
          _stack[sp] = 1.0 / _stack[sp];
        break;
      }

      default:
        fatalError("Invalid opcode " + stringify(_byte_code[ip]) + " at ip=" + stringify(ip) +
                   " sp=" + stringify(sp));
    }
  } while (++ip < byte_code_size);

  // return result from top of stack
  return _stack[sp];
}

template <typename T>
void
CompiledByteCode<T>::print()
{
  // disassemble the bytecode
  static const std::vector<std::string> instruction = {"LOAD_IMMEDIATE_INTEGER",
                                                       "LOAD_IMMEDIATE_REAL",
                                                       "LOAD_VARIABLE_REAL",
                                                       "UO_PLUS",
                                                       "UO_MINUS",
                                                       "UO_FACULTY",
                                                       "UO_NOT",
                                                       "BO_SUBTRACTION",
                                                       "BO_DIVISION",
                                                       "BO_MODULO",
                                                       "BO_POWER",
                                                       "BO_LOGICAL_OR",
                                                       "BO_LOGICAL_AND",
                                                       "BO_LESS_THAN",
                                                       "BO_GREATER_THAN",
                                                       "BO_LESS_EQUAL",
                                                       "BO_GREATER_EQUAL",
                                                       "BO_EQUAL",
                                                       "BO_NOT_EQUAL",
                                                       "BO_ASSIGNMENT",
                                                       "BO_LIST",
                                                       "MO_ADDITION",
                                                       "MO_MULTIPLICATION",
                                                       "MO_COMPONENT",
                                                       "MO_LIST",
                                                       "UF_ABS",
                                                       "UF_ACOS",
                                                       "UF_ACOSH",
                                                       "UF_ARG",
                                                       "UF_ASIN",
                                                       "UF_ASINH",
                                                       "UF_ATAN",
                                                       "UF_ATANH",
                                                       "UF_CBRT",
                                                       "UF_CEIL",
                                                       "UF_CONJ",
                                                       "UF_COS",
                                                       "UF_COSH",
                                                       "UF_COT",
                                                       "UF_CSC",
                                                       "UF_ERF",
                                                       "UF_ERFC",
                                                       "UF_EXP",
                                                       "UF_EXP2",
                                                       "UF_FLOOR",
                                                       "UF_IMAG",
                                                       "UF_INT",
                                                       "UF_LOG",
                                                       "UF_LOG10",
                                                       "UF_LOG2",
                                                       "UF_REAL",
                                                       "UF_SEC",
                                                       "UF_SIN",
                                                       "UF_SINH",
                                                       "UF_SQRT",
                                                       "UF_T",
                                                       "UF_TAN",
                                                       "UF_TANH",
                                                       "UF_TRUNC",
                                                       "BF_ATAN2",
                                                       "BF_HYPOT",
                                                       "BF_MAX",
                                                       "BF_MIN",
                                                       "BF_PLOG",
                                                       "BF_POLAR",
                                                       "BF_POW",
                                                       "CONDITIONAL",
                                                       "INTEGER_POWER",
                                                       "JUMP"};

  for (std::size_t i = 0; i < _byte_code.size(); ++i)
  {
    auto vc = _byte_code[i];
    auto vi = static_cast<VMInstruction>(vc);

    std::cout << i << " [" << vc << "] " << instruction[vc] << '\n';

    switch (vi)
    {
      case VMInstruction::LOAD_IMMEDIATE_REAL:
        ++i;
        std::cout << i << " [" << _byte_code[i] << "] " << _immed[_byte_code[i]] << '\n';
        break;

      case VMInstruction::LOAD_VARIABLE_REAL:
        ++i;
        std::cout << i << " [" << _byte_code[i] << "] " << *_vars[_byte_code[i]] << '\n';
        break;

      case VMInstruction::CONDITIONAL:
      case VMInstruction::INTEGER_POWER:
      case VMInstruction::JUMP:
        ++i;
        std::cout << i << " [" << _byte_code[i] << "] " << '\n';
        break;

      default:
        break;
    }
  }
}

template class CompiledByteCode<Real>;

} // namespace SymbolicMath
