#!/bin/bash
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
[ "x$1" == "x" ] && OUT="/tmp/${RANDOM}_${RANDOM}_${RANDOM}" || OUT="$1"
awk 'BEGIN{x=0;}{if (x == 1 || $0 == "///") { x = 1; print; }}' $0 | \
gcc -S -fPIC -pie -std=gnu99 -o "${OUT}.s" -x c - && \
    awk '
        BEGIN{FS="\t"}
        {
            print;
            if ($2 == "call") {
                #// must match RETURN_MAGIC
                #// byte 0xeb ; jmp
                #// byte 0x02 ; +2 bytes
                #// byte 0xfe ; arbitrary
                #// byte 0xca ; arbitrary
                print("\t.byte 0xeb\n\t.byte 0x02\n\t.byte 0xfe\n\t.byte 0xca");
            }
        }' "${OUT}.s" > "${OUT}_pp.s" && \
    gcc -o "${OUT}" "${OUT}_pp.s" && \
    ${OUT}
    exit $?
///

#include <stdio.h>

#define RETURN_MAGIC 0xcafe02eb

void __stack_chk_fail();

#define safeReturn(...) \
    do {                                                                                    \
        if ( ((unsigned int*)__builtin_return_address(0) )[0] != RETURN_MAGIC ) {           \
            __stack_chk_fail();                                                             \
        }                                                                                   \
        return __VA_ARGS__ ;                                                                \
    } while (0)

static int test(int input)
{
    if (input) {
        printf("It works!\n");
    }
    safeReturn(0);
}


int main()
{
    test(0);
    test(1);
}
