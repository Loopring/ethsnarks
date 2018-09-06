// Copyright (c) 2018 HarryR
// License: LGPL-3.0+

#include <libsnark/gadgetlib1/gadgets/basic_gadgets.hpp>

using libsnark::gadget;
using libsnark::pb_variable;
using libsnark::pb_variable_array;
using libsnark::protoboard;
using libsnark::r1cs_constraint;


#include "longsightf.hpp"

using ethsnarks::FieldT;

/**
* The LongsightF function can be represented as a circuit:
*
*         L       R
*        x_1     x_0
*         _       _
*         |       |
*         |--------------------.
*         |       |            |
*         v       |            |
* C_0 |->(+)      |            |    j[i] = x[i+1] + C[i]
*         |       |            |
*         v       |            | 
*       (^5)      |            |    k[i] = j[i]^5
*         |       v            |
*          `---->(+) = x_2     |  x[i+2] = x[i] + k[i]
*                      _       |
*                      |       |
*                      |--------------------.
*                      |       |            |
*                      v       |            |
*              C_i |->(+)      |            |
*                      |       |            |
*                      v       |            |
*                    (^5)      |            |
*                      |       v            |
*                      `----->(+) = x_(i+2) |
*                                   _       |
*                                   |       |
*                                   v       |
*                       C_(i-1) |->(+)      |
*                                   |       |
*                                   v       |
*                                  (^5)     |
*                                   |       v
*                                   `----->(+) = output
*
*  The round function can be expressed as:
*
*       x[i+2] = x[i] + (x[i+1] + C[i])^5
*
*  Where x[] must start with at least 2 values
*
*  If the values x[0] and x[1] are the variables L and R
*  and x[] is going to be the intermediate state of the function
*  then the first two rounds must substitute those variables, e.g.
*
*       x[0] = R      + (L      + C[i])^5          when i = 0
*       x[1] = L      + (x[i-1] + C[i])^5          when i = 1
*       x[i] = x[i-2] + (x[i-1] + C[i])^5          when i > 1
*
*       output = x[ len(x) - 1 ]
*
*  Knowing the value of x2, x1 and C then x0 can be easily found, while
*  only knowing x0, C and the result finding x1 isn't as trivial.
*
* (%i1) solve([ x[2] = x[0] + (C+x[1])^5 ], [x[2]]);
*
*                 5         4       2  3       3  2      4      5
* (%o1)    [x  = C  + 5 x  C  + 10 x  C  + 10 x  C  + 5 x  C + x  + x ]
*            2           1          1          1         1      1    0
*
*/


LongsightF_gadget::LongsightF_gadget(
    protoboard<FieldT> &in_pb,
    const std::vector<FieldT> in_constants,
    const pb_variable<FieldT> in_x_L,
    const pb_variable<FieldT> in_x_R,
    const std::string &in_annotation_prefix,
    const bool do_allocate
) :
    gadget<FieldT>(in_pb, FMT(in_annotation_prefix, " LongsightF_gadget")),
    round_constants(in_constants),
    start_L(in_x_L),
    start_R(in_x_R),
    round_squares(),
    rounds()
{
    // Constants may be initialised after constructor
    // Allow allocation to happen separately
    if( do_allocate ) {
        this->allocate();
    }
}

void LongsightF_gadget::allocate()
{
    round_squares.allocate(this->pb, round_constants.size() * 2, FMT(this->annotation_prefix, " round_squares"));

    rounds.allocate(this->pb, round_constants.size(), FMT(this->annotation_prefix, " rounds"));
}

const pb_variable<FieldT>& LongsightF_gadget::result() const
{
    return rounds[ rounds.size() - 1 ];
}

void LongsightF_gadget::generate_r1cs_constraints()
{
    size_t j = 0;

    for( size_t i = 0; i < round_constants.size() - 2; i++ )
    {
        const pb_variable<FieldT>& xL = (
            i == 0 ? start_L
                   : rounds[i-1]);

        const pb_variable<FieldT>& xR = (
            i == 0 ? start_R
                   : (i == 1 ? start_L
                             : rounds[i-2]));

        // -------------------------------------------------
        // Squares

            // (xL+C[i]) * (xL+C[i]) = j[1]
            this->pb.add_r1cs_constraint(
                r1cs_constraint<FieldT>(
                    round_constants[i] + xL,
                    round_constants[i] + xL,
                    round_squares[j]));

            // j[1] * (xL+C[i]) = j[2]
            this->pb.add_r1cs_constraint(
                r1cs_constraint<FieldT>(
                    round_squares[j],
                    round_squares[j],
                    round_squares[j+1]));

        // -------------------------------------------------
        // Intermediate outputs

            // j[2] * (xL+C[i]) = x[i] - xR
            this->pb.add_r1cs_constraint(
                r1cs_constraint<FieldT>(
                    round_squares[j+1],
                    round_constants[i] + xL,
                    rounds[i] - xR));

        // -------------------------------------------------
        // Move to next block of squares
        j += 2;
    }        
}

void LongsightF_gadget::generate_r1cs_witness()
{
    size_t h = 0;
    for( size_t i = 0; i < round_constants.size(); i++ )
    {
        const FieldT& xR = (
            i == 0 ? this->pb.val(start_R)
                   : (i == 1 ? this->pb.val(start_L)
                             : this->pb.val(rounds[i-2])));

        const FieldT& xL = (i == 0 ? this->pb.val(start_L) : this->pb.val(rounds[i-1]));

        // Intermediate squarings
        auto t = xL + round_constants[i];
        this->pb.val(round_squares[h]) = t * t;        // ^2  
        this->pb.val(round_squares[h+1]) = this->pb.val(round_squares[h]) * this->pb.val(round_squares[h]);    // ^4

        // Then intermediate X point
        this->pb.val(rounds[i]) = xR + (this->pb.val(round_squares[h+1]) * t);

        // Next block of intermediate squarings
        h += 2;
    }
}
