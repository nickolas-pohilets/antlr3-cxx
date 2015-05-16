/*
 * [The "BSD license"]
 *  Copyright (c) 2010 Terence Parr
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package org.antlr.codegen;

import org.antlr.Tool;
import org.antlr.tool.Grammar;
import org.antlr.tool.Interp;
import org.antlr.tool.TextEncoder;
import org.stringtemplate.v4.ST;
import org.stringtemplate.v4.misc.Aggregate;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

public class CxxTarget extends Target {

    @Override
    protected void genRecognizerFile(Tool tool,
            CodeGenerator generator,
            Grammar grammar,
            ST outputFileST)
            throws IOException {
        registerNamespaceAttributes(grammar, outputFileST);
        String fileName = generator.getRecognizerFileName(grammar.name, grammar.type);
        generator.write(outputFileST, fileName);
    }

    @Override
    protected void genRecognizerHeaderFile(Tool tool,
            CodeGenerator generator,
            Grammar grammar,
            ST headerFileST,
            String extName)
            throws IOException {

        registerNamespaceAttributes(grammar, headerFileST);
        
        //Its better we remove the EOF Token, as it would have been defined everywhere in C.
        //we define it later as "EOF_TOKEN" instead of "EOF"
        List<?> tokens = (List<?>)headerFileST.getAttribute("tokens");
        for( int i = 0; i < tokens.size(); ++i )
        {
            boolean can_break = false;
            Object tok = tokens.get(i);
            if( tok instanceof Aggregate )
            {
                Aggregate atok = (Aggregate) tok;
                for (Map.Entry<String, Object> pairs : atok.properties.entrySet()) {
                    if( pairs.getValue().equals("EOF") )
                    {
                        tokens.remove(i);
                        can_break = true;
                        break;
                    }
                }
            }

            if( can_break )
                break;
        }

        // Pick up the file name we are generating. This method will return a
        // a file suffixed with .cpp, so we must substring and add the extName
        // to it as we cannot assign into strings in Java.
        ///
        String fileName = generator.getRecognizerFileName(grammar.name, grammar.type);
        fileName = fileName.substring(0, fileName.length() - 4) + extName;
        System.err.println("writing: " + fileName);
        generator.write(headerFileST, fileName);
    }

    protected ST chooseWhereCyclicDFAsGo(Tool tool,
            CodeGenerator generator,
            Grammar grammar,
            ST recognizerST,
            ST cyclicDFAST) {
        return recognizerST;
    }

    /** Is scope in @scope::name {action} valid for this kind of grammar?
     *  Targets like C++ may want to allow new scopes like headerfile or
     *  some such.  The action names themselves are not policed at the
     *  moment so targets can add template actions w/o having to recompile
     *  ANTLR.
     */
    @Override
    public boolean isValidActionScope(int grammarType, String scope) {
        switch (grammarType) {
            case Grammar.LEXER:
                if (scope.equals("lexer")) {
                    return true;
                }
                if (scope.equals("header")) {
                    return true;
                }
                if (scope.equals("includes")) {
                    return true;
                }
                if (scope.equals("preincludes")) {
                    return true;
                }
                if (scope.equals("overrides")) {
                    return true;
                }
                break;
            case Grammar.PARSER:
                if (scope.equals("parser")) {
                    return true;
                }
                if (scope.equals("header")) {
                    return true;
                }
                if (scope.equals("includes")) {
                    return true;
                }
                if (scope.equals("preincludes")) {
                    return true;
                }
                if (scope.equals("overrides")) {
                    return true;
                }
                break;
            case Grammar.COMBINED:
                if (scope.equals("parser")) {
                    return true;
                }
                if (scope.equals("lexer")) {
                    return true;
                }
                if (scope.equals("header")) {
                    return true;
                }
                if (scope.equals("includes")) {
                    return true;
                }
                if (scope.equals("preincludes")) {
                    return true;
                }
                if (scope.equals("overrides")) {
                    return true;
                }
                break;
            case Grammar.TREE_PARSER:
                if (scope.equals("treeparser")) {
                    return true;
                }
                if (scope.equals("header")) {
                    return true;
                }
                if (scope.equals("includes")) {
                    return true;
                }
                if (scope.equals("preincludes")) {
                    return true;
                }
                if (scope.equals("overrides")) {
                    return true;
                }
                break;
        }
        return false;
    }

    public boolean supportsEncoding(String encoding) {
        return true;
    }

    private String stringLiteralPrefix() {
        String encoding = getTextEncoder().getEncoding();
        if (encoding.equals(TextEncoder.UTF8_ENCODING)) {
            return "u8";
        }
        if (encoding.equals(TextEncoder.UTF16_ENCODING)) {
            return "u";
        }
        if (encoding.equals(TextEncoder.UTF32_ENCODING)) {
            return "U";
        }
        assert false;
        return "";
    }

    @Override
    public String getTargetCharLiteralFromANTLRCharLiteral(
            CodeGenerator generator,
            String literal) {
        StringBuffer unescaped = Grammar.getUnescapedStringFromGrammarStringLiteral(literal);
        int c = Grammar.getCharValueFromUnescapedString(unescaped, literal);
        if (c < 0 ) {
            return "0";
        }

        String esc = null;
        if (c < targetCharValueEscape.length && targetCharValueEscape[c]!=null) {
            esc = targetCharValueEscape[c];
        } else if (c < 0x20 || c >= 0x80) {
            int maxChar = getTextEncoder().getMaxCodeValue();
            int padding = maxChar < 0x100 ? 2 : 4;
            String retVal = Integer.toHexString(c).toUpperCase();
            int leadingZeroes = retVal.length() < padding ? padding - retVal.length() : 0;
            return "0x" + "00000000".substring(0, leadingZeroes) + retVal;
        }

        StringBuilder buf = new StringBuilder();
        buf.append('\'');
        if (esc != null) {
            buf.append(esc);
        }
        else {
            // normal char
            buf.append((char)c);
        }
        buf.append('\'');
        return buf.toString();
    }

    /** Convert from an ANTLR string literal found in a grammar file to
     *  an equivalent string literal in the C target.
     *  Because we must support Unicode character sets and have chosen
     *  to have the lexer match UTF32 characters, then we must encode
     *  string matches to use 32 bit character arrays. Here then we
     *  must produce the C array and cater for the case where the
     *  lexer has been encoded with a string such as 'xyz\n',
     */
    @Override
    public String getTargetStringLiteralFromANTLRStringLiteral(
            CodeGenerator generator,
            String literal) {
        return stringLiteralPrefix() + super.getTargetStringLiteralFromANTLRStringLiteral(generator, literal);
    }

    /**
     * Overrides the standard grammar analysis so we can prepare the analyser
     * a little differently from the other targets.
     *
     * In particular we want to influence the way the code generator makes assumptions about
     * switchs vs ifs, vs table driven DFAs. In general, C code should be generated that
     * has the minimum use of tables, and tha meximum use of large switch statements. This
     * allows the optimizers to generate very efficient code, it can reduce object code size
     * by about 30% and give about a 20% performance improvement over not doing this. Hence,
     * for the C target only, we change the defaults here, but only if they are still set to the
     * defaults.
     *
     * @param generator An instance of the generic code generator class.
     * @param grammar The grammar that we are currently analyzing
     */
    @Override
    protected void performGrammarAnalysis(CodeGenerator generator, Grammar grammar) {

        // Check to see if the maximum inline DFA states is still set to
        // the default size. If it is then whack it all the way up to the maximum that
        // we can sensibly get away with.
        //
        if (CodeGenerator.MAX_ACYCLIC_DFA_STATES_INLINE == CodeGenerator.MADSI_DEFAULT ) {

            CodeGenerator.MAX_ACYCLIC_DFA_STATES_INLINE = 65535;
        }

        // Check to see if the maximum switch size is still set to the default
        // and bring it up much higher if it is. Modern C compilers can handle
        // much bigger switch statements than say Java can and if anyone finds a compiler
        // that cannot deal with such big switches, all the need do is generate the
        // code with a reduced -Xmaxswitchcaselabels nnn
        //
        if  (CodeGenerator.MAX_SWITCH_CASE_LABELS == CodeGenerator.MSCL_DEFAULT) {

            CodeGenerator.MAX_SWITCH_CASE_LABELS = 3000;
        }

        // Check to see if the number of transitions considered a miminum for using
        // a switch is still at the default. Because a switch is still generally faster than
        // an if even with small sets, and given that the optimizer will do the best thing with it
        // anyway, then we simply want to generate a switch for any number of states.
        //
        if (CodeGenerator.MIN_SWITCH_ALTS == CodeGenerator.MSA_DEFAULT) {

            CodeGenerator.MIN_SWITCH_ALTS = 1;
        }

        // Now we allow the superclass implementation to do whatever it feels it
        // must do.
        //
        super.performGrammarAnalysis(generator, grammar);
    }

    private void registerNamespaceAttributes(Grammar g, ST st) {
        if (g.composite != null) {
            g = g.composite.getRootGrammar();
        }

        String actionScope = g.getDefaultActionScope(g.type);
        Map<String, Object> map = g.getActions().get(actionScope);
        if (map == null) return;
        Object val = map.get("namespace");
        if (val == null) return;

        String ns = String.join("", (ArrayList<String>)val);
        String[] namespaceComponets = ns.split("::");
        for (int i = 0; i < namespaceComponets.length; ++i) {
            namespaceComponets[i] = namespaceComponets[i].trim();
        }
        st.add("namespaceComponents", Arrays.asList(namespaceComponets));
    }
}

