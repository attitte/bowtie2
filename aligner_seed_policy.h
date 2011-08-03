/*
 *  aligner_seed_policy.h
 */

#ifndef ALIGNER_SEED_POLICY_H_
#define ALIGNER_SEED_POLICY_H_

#include "scoring.h"
#include "simple_func.h"

#define DEFAULT_SEEDMMS 0
#define DEFAULT_SEEDLEN 22

#define DEFAULT_IVAL SIMPLE_FUNC_SQRT
#define DEFAULT_IVAL_A 1.0f
#define DEFAULT_IVAL_B 0.0f

// Maximum number of positions we examine.  For example the seed extracted
// from 5' offset 10 of the forward representation of the read might be
// one position, and the same from the reverse-complement representation
// might be another.
#define DEFAULT_POSMIN  1.5f
#define DEFAULT_POSFRAC 0.15f

// The maximum number of hits we try to extend for any given position.
#define DEFAULT_ROWMULT  1.0f

/**
 * Encapsulates the set of all parameters that affect what the
 * SeedAligner does with reads.
 */
class SeedAlignmentPolicy {

public:

	/**
	 * Parse alignment policy when provided in this format:
	 * <lab>=<val>;<lab>=<val>;<lab>=<val>...
	 *
	 * And label=value possibilities are:
	 *
	 * Bonus for a match
	 * -----------------
	 *
	 * MA=xx (default: MA=0, or MA=2 if --local is set)
	 *
	 *    xx = Each position where equal read and reference characters match up
	 *         in the alignment contriubtes this amount to the total score.
	 *
	 * Penalty for a mismatch
	 * ----------------------
	 *
	 * MMP={Cxx|Q|RQ} (default: MMP=C6)
	 *
	 *   Cxx = Each mismatch costs xx.  If MMP=Cxx is specified, quality
	 *         values are ignored when assessing penalities for mismatches.
	 *   Q   = Each mismatch incurs a penalty equal to the mismatched base's
	 *         value.
	 *   R   = Each mismatch incurs a penalty equal to the mismatched base's
	 *         rounded quality value.  Qualities are rounded off to the
	 *         nearest 10, and qualities greater than 30 are rounded to 30.
	 *
	 * Penalty for a SNP in a colorspace alignment
	 * -------------------------------------------
	 *
	 * SNP=xx (default: SNP=6)
	 *
	 *    xx = Each nucleotide difference in a decoded colorspace alignment
	 *         costs xx.  This should be about equal to -10 * log10(expected
	 *         fraction of positions that are SNPs)
	 * 
	 * Penalty for position with N (in either read or reference)
	 * ---------------------------------------------------------
	 *
	 * NP={Cxx|Q|RQ} (default: NP=C1)
	 *
	 *   Cxx = Each alignment position with an N in either the read or the
	 *         reference costs xx.  If NP=Cxx is specified, quality values are
	 *         ignored when assessing penalities for Ns.
	 *   Q   = Each alignment position with an N in either the read or the
	 *         reference incurs a penalty equal to the read base's quality
	 *         value.
	 *   R   = Each alignment position with an N in either the read or the
	 *         reference incurs a penalty equal to the read base's rounded
	 *         quality value.  Qualities are rounded off to the nearest 10,
	 *         and qualities greater than 30 are rounded to 30.
	 *
	 * Penalty for a read gap
	 * ----------------------
	 *
	 * RDG=xx,yy (default: RDG=5,3)
	 *
	 *   xx    = Read gap open penalty.
	 *   yy    = Read gap extension penalty.
	 *
	 * Total cost incurred by a read gap = xx + (yy * gap length)
	 *
	 * Penalty for a reference gap
	 * ---------------------------
	 *
	 * RFG=xx,yy (default: RFG=5,3)
	 *
	 *   xx    = Reference gap open penalty.
	 *   yy    = Reference gap extension penalty.
	 *
	 * Total cost incurred by a reference gap = xx + (yy * gap length)
	 *
	 * Minimum score for valid alignment
	 * ---------------------------------
	 *
	 * MIN=xx,yy (defaults: MIN=-0.6,-0.6, or MIN=0.0,0.66 if --local is set)
	 *
	 *   xx,yy = For a read of length N, the total score must be at least
	 *           xx + (read length * yy) for the alignment to be valid.  The
	 *           total score is the sum of all negative penalties (from
	 *           mismatches and gaps) and all positive bonuses.  The minimum
	 *           can be negative (and is by default in global alignment mode).
	 *
	 * Score floor for local alignment
	 * -------------------------------
	 *
	 * FL=xx,yy (defaults: FL=-Infinity,0.0, or FL=0.0,0.0 if --local is set)
	 *
	 *   xx,yy = If a cell in the dynamic programming table has a score less
	 *           than xx + (read length * yy), then no valid alignment can go
	 *           through it.  Defaults are highly recommended.
	 *
	 * N ceiling
	 * ---------
	 *
	 * NCEIL=xx,yy (default: NCEIL=0.0,0.15)
	 *
	 *   xx,yy = For a read of length N, the number of alignment
	 *           positions with an N in either the read or the
	 *           reference cannot exceed
	 *           ceiling = xx + (read length * yy).  If the ceiling is
	 *           exceeded, the alignment is considered invalid.
	 *
	 * Seeds
	 * -----
	 *
	 * SEED=mm,len,ival (default: SEED=0,22)
	 *
	 *   mm   = Maximum number of mismatches allowed within a seed.
	 *          Must be >= 0 and <= 2.  Note that 2-mismatch mode is
	 *          not fully sensitive; i.e. some 2-mismatch seed
	 *          alignments may be missed.
	 *   len  = Length of seed.
	 *   ival = Interval between seeds.  If not specified, seed
	 *          interval is determined by IVAL.
	 *
	 * Seed interval
	 * -------------
	 *
	 * IVAL={L|S|C},xx,yy (default: IVAL=S,1.0,0.0)
	 *
	 *   L  = let interval between seeds be a linear function of the
	 *        read length.  xx and yy are the constant and linear
	 *        coefficients respectively.  In other words, the interval
	 *        equals a * len + b, where len is the read length.
	 *        Intervals less than 1 are rounded up to 1.
	 *   S  = let interval between seeds be a function of the sqaure
	 *        root of the  read length.  xx and yy are the
	 *        coefficients.  In other words, the interval equals
	 *        a * sqrt(len) + b, where len is the read length.
	 *        Intervals less than 1 are rounded up to 1.
	 *   C  = Like S but uses cube root of length instead of square
	 *        root.
	 *
	 * Example 1:
	 *
	 *  SEED=1,10,5 and read sequence is TGCTATCGTACGATCGTAC:
	 *
	 *  The following seeds are extracted from the forward
	 *  representation of the read and aligned to the reference
	 *  allowing up to 1 mismatch:
	 *
	 *  Read:    TGCTATCGTACGATCGTACA
	 *
	 *  Seed 1+: TGCTATCGTA
	 *  Seed 2+:      TCGTACGATC
	 *  Seed 3+:           CGATCGTACA
	 *
	 *  ...and the following are extracted from the reverse-complement
	 *  representation of the read and align to the reference allowing
	 *  up to 1 mismatch:
	 *
	 *  Seed 1-: TACGATAGCA
	 *  Seed 2-:      GATCGTACGA
	 *  Seed 3-:           TGTACGATCG
	 *
	 * Example 2:
	 *
	 *  SEED=1,20,20 and read sequence is TGCTATCGTACGATC.  The seed
	 *  length is 20 but the read is only 15 characters long.  In this
	 *  case, Bowtie2 automatically shrinks the seed length to be equal
	 *  to the read length.
	 *
	 *  Read:    TGCTATCGTACGATC
	 *
	 *  Seed 1+: TGCTATCGTACGATC
	 *  Seed 1-: GATCGTACGATAGCA
	 *
	 * Example 3:
	 *
	 *  SEED=1,10,10 and read sequence is TGCTATCGTACGATC.  Only one seed
	 *  fits on the read; a second seed would overhang the end of the read
	 *  by 5 positions.  In this case, Bowtie2 extracts one seed.
	 *
	 *  Read:    TGCTATCGTACGATC
	 *
	 *  Seed 1+: TGCTATCGTA
	 *  Seed 1-: TACGATAGCA
	 */
	static void parseString(
		const  std::string& s,
		bool   local,
		bool   noisyHpolymer,
		int&   bonusMatchType,
		int&   bonusMatch,
		int&   penMmcType,
		int&   penMmc,
		int&   penSnp,
		int&   penNType,
		int&   penN,
		int&   penRdExConst,
		int&   penRfExConst,
		int&   penRdExLinear,
		int&   penRfExLinear,
		SimpleFunc& costMin,
		SimpleFunc& costFloor,
		SimpleFunc& nCeil,
		bool&  nCatPair,
		int&   multiseedMms,
		int&   multiseedLen,
		SimpleFunc& multiseedIval,
		SimpleFunc& posmin,
		SimpleFunc& rowmult);
};

#endif /*ndef ALIGNER_SEED_POLICY_H_*/
