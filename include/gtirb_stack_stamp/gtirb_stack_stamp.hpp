/*******************************************************************************
Copyright (C) 2020 GrammaTech, Inc.

This code is licensed under the MIT license. See the LICENSE file in
the project root for license terms.

This project is sponsored by the Office of Naval Research, One Liberty
Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
N68335-17-C-0700.  The content of the information does not necessarily
reflect the position or policy of the Government and no official
endorsement should be inferred.
*******************************************************************************/

#ifndef GTIRB_STACK_STAMP_H
#define GTIRB_STACK_STAMP_H

#include <gtirb/gtirb.hpp>
#include <capstone/capstone.h>
#include <keystone/keystone.h>

#include <cassert>
#include <string>

namespace gtirb_stack_stamp {

/// \class StackStamper
///
/// \brief This class handles the insertion of stack-stamps in the entry and
/// exit blocks of functions, as a simple example of control-flow integrity.
class StackStamper {
public:
  explicit StackStamper(gtirb::Context& Ctx_) : Ctx{Ctx_} {
    [[maybe_unused]] auto CSRet = cs_open(CS_ARCH_X86, CS_MODE_64, &Capstone);
    assert(CSRet == CS_ERR_OK);
    cs_option(Capstone, CS_OPT_DETAIL, CS_OPT_ON);

    [[maybe_unused]] auto KSRet =
        ks_open(KS_ARCH_X86, KS_MODE_LITTLE_ENDIAN | KS_MODE_64, &Keystone);
    assert(KSRet == KS_ERR_OK);
    ks_option(Keystone, KS_OPT_SYNTAX, KS_OPT_SYNTAX_ATT);
  }

  ~StackStamper() {
    cs_close(&Capstone);
    ks_close(Keystone);
  }

  /// \brief Insert a sequence of assembly instructions into a byte interval.
  /// Unlike ByteInterval::insertBytes, it automatically updates the offsets
  /// of anything that occurs after the insertion.
  ///
  /// \param BI The byte interval to insert into.
  /// \param Offset The offset into the byte interval to insert code at.
  /// \param InsnsStr A string representing a sequence of assembly instructions.
  void insertInstructions(gtirb::ByteInterval& BI, uint64_t Offset,
                          const std::string& InsnsStr) const;

  /// \brief Insert stack-stamping instructions into the entrance block of a
  /// function.
  ///
  /// \param FunctionId The UUID of the function we are stack-stamping. Used
  /// to calculate hash values.
  /// \param Block The code block to insert instructions into.
  void stampEntranceBlock(const gtirb::UUID& FunctionId,
                          gtirb::CodeBlock& Block) const;

  /// \brief Insert stack-stamping instructions into the exit block of a
  /// function, that is, right before a return statement.
  ///
  /// \param FunctionId The UUID of the function we are stack-stamping. Used
  /// to calculate hash values.
  /// \param Block The code block to insert instructions into.
  void stampExitBlock(const gtirb::UUID& FunctionId,
                      gtirb::CodeBlock& Block) const;

  /// \brief Stack-stamp a function.
  ///
  /// \param M The module that contains the function.
  /// \param FunctionId The UUID of the function to stack-stamp.
  void stampFunction(gtirb::Module& M, const gtirb::UUID& FunctionId) const;

  /// \brief Is this code block an exit block; that is, does it end in a
  /// return instruction?
  bool isExitBlock(const gtirb::CodeBlock& Block) const;

private:
  gtirb::Context& Ctx;
  csh Capstone;
  ks_engine* Keystone;
};

/// \brief Stack-stamps all functions in a module.
///
/// \param Ctx the context that the module was created from.
/// \param M The module to stack-stamp.
void stamp(gtirb::Context& Ctx, gtirb::Module& M);

/// \brief Registers all auxillary data schema needed by this file.
/// Call this function before any usages of the GTIRB API.
void registerAuxDataSchema();

} // namespace gtirb_stack_stamp

#endif // GTIRB_STACK_STAMP_H
