/** Support functions for traversing cyclic DFA states as laid
 *  out in static initialized structures by the code generator.
 *
 * A DFA implemented as a set of transition tables.
 *
 *  Any state that has a semantic predicate edge is special; those states
 *  are generated with if-then-else structures in a ->specialStateTransition()
 *  which is generated by cyclicDFA template.
 *
 *  There are at most 32767 states (16-bit signed short).
 *  Could get away with byte sometimes but would have to generate different
 *  types and the simulation code too.  For a point of reference, the Java
 *  lexer's Tokens rule DFA has 326 states roughly.
 */

// [The "BSD licence"]
// Copyright (c) 2005-2009 Jim Idle, Temporal Wave LLC
// http://www.temporal-wave.com
// http://www.linkedin.com/in/jimidle
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <antlr3/Defs.hpp>
#include <antlr3/CyclicDFA.hpp>

namespace antlr3 {

#ifdef	Windows
#pragma warning( disable : 4100 )
#endif

void CyclicDfa::noViableAlt(BaseRecognizer * rec, std::uint32_t	s) const
{
	// In backtracking mode, we just set the failed flag so that the
	// alt can just exit right now. If we are parsing though, then 
	// we want the exception to be raised.
	//
    if	(rec->state_->backtracking > 0)
    {
		rec->state_->failed = true;
    }
	else
	{
        rec->recordException(new NoViableAltException(description, decisionNumber, s));
	}
}

/** From the input stream, predict what alternative will succeed
 *  using this DFA (representing the covering regular approximation
 *  to the underlying CFL).  Return an alternative number 1..n.  Throw
 *  an exception upon error.
 */
std::int32_t CyclicDfa::predict(void * ctx, BaseRecognizer * rec, IntStream * is) const
{
    MarkerPtr mark = is->mark();	    /* Store where we are right now	*/
    std::int32_t s		= 0;		    /* Always start with state 0	*/
    
	for (;;)
	{
		/* Pick out any special state entry for this state
		 */
		std::int32_t specialState = special[s];

		/* Transition the special state and consume an input token
		 */
		if  (specialState >= 0)
		{
			s = specialStateTransition(ctx, rec, is, specialState, mark);

			// Error?
			//
			if	(s<0)
			{
				// If the predicate/rule raised an exception then we leave it
				// in tact, else we have an NVA.
				//
				if	(!rec->state_->error)
				{
					noViableAlt(rec, s);
				}
				mark->rewind();
				return 0;
			}
			is->consume();
			continue;
		}

		/* Accept state?
		 */
		if  (accept[s] >= 1)
		{
			mark->rewind();
			return  accept[s];
		}

		/* Look for a normal transition state based upon the input token element
		 */
		std::int32_t c = is->LA(1);

		/* Check against min and max for this state
		 */
		if  (c>= min[s] && c <= max[s])
		{
			std::int32_t   snext;

			/* What is the next state?
			 */
			snext = transition[s][c - min[s]];

			if	(snext < 0)
			{
				/* Was in range but not a normal transition
				 * must check EOT, which is like the else clause.
				 * eot[s]>=0 indicates that an EOT edge goes to another
				 * state.
				 */
				if  (eot[s] >= 0)
				{
					s = eot[s];
					is->consume();
					continue;
				}
				noViableAlt(rec, s);
				mark->rewind();
				return	0;
			}

			/* New current state - move to it
			 */
			s = snext;
			is->consume();
			continue;
		}
		/* EOT Transition?
		 */
		if  (eot[s] >= 0)
		{
			s	= eot[s];
			is->consume();
			continue;
		}
		/* EOF transition to accept state?
		 */
		if(c == TokenEof && eof[s] >= 0)
		{
			mark->rewind();
			return  accept[eof[s]];
		}

		/* No alt, so bomb
		 */
		noViableAlt(rec, s);
		mark->rewind();
		return 0;
	}

}

/** Default special state implementation
 */
std::int32_t CyclicDfa::specialStateTransition(void * ctx, BaseRecognizer * recognizer, IntStream * is, std::int32_t s, MarkerPtr marker) const
{
    if (specialStateTransitionFunc == NULL)
    {
        return -1;
    }
    else
    {
        return (*specialStateTransitionFunc)(ctx, recognizer, is, s, marker);
    }
}

} // namespace antlr3
