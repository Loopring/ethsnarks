import unittest
from os import urandom
from random import randint

from ethsnarks.jubjub import Point, FQ
from ethsnarks.pedersen import pedersen_hash_bytes, pedersen_hash_bits, pedersen_hash_scalars


class TestPedersenHash(unittest.TestCase):
    def test_zcash(self):
        d = randint(1, 1024)
        p = pedersen_hash_scalars(b'test', d)
        q = pedersen_hash_scalars(b'test', d, d)
        self.assertTrue(p.valid)
        self.assertTrue(q.valid)
        self.assertNotEqual(p, q)

    def test_hash_scalars_known(self):
        self.assertEqual(
            pedersen_hash_scalars(b'test', 267),
            Point(FQ(6790798216812059804926342266703617627640027902964190490794793207272357201212),
                  FQ(2522797517250455013248440571887865304858084343310097011302610004060289809689)))

        self.assertEqual(
            pedersen_hash_scalars(b'test', 6453482891510615431577168724743356132495662554103773572771861111634748265227),
            Point(FQ(6545697115159207040330446958704617656199928059562637738348733874272425400594),
                  FQ(16414097465381367987194277536478439232201417933379523927469515207544654431390)))

        self.assertEqual(
            pedersen_hash_scalars(b'test', 21888242871839275222246405745257275088548364400416034343698204186575808495616),
            Point(FQ(16322787121012335146141962340685388833598805940095898416175167744309692564601),
                  FQ(7671892447502767424995649701270280747270481283542925053047237428072257876309)))

    def test_hash_bytes_known(self):
        self.assertEqual(
            pedersen_hash_bytes(b'test', b"abc"),
            Point(FQ(9869277320722751484529016080276887338184240285836102740267608137843906399765),
                  FQ(19790690237145851554496394080496962351633528315779989340140084430077208474328)))
    
        self.assertEqual(
            pedersen_hash_bytes(b'test', b"abcdefghijklmnopqrstuvwx"),
            Point(FQ(3966548799068703226441887746390766667253943354008248106643296790753369303077),
                  FQ(12849086395963202120677663823933219043387904870880733726805962981354278512988)))

    def test_hash_bits_known(self):
        self.assertEqual(
            pedersen_hash_bits('EdDSA_Verify.RAM', '101100110011111001100100101100010100011010100100001011101001000100100000001111101101111001001010111011101101011010010101101101101000000010000000101010110100011110101110111100111100011110110011100101011000000000110101111001110000101011011110100100011110010000110111010011000001000100101100101111001100100010110101100010001000000101111011011010010011110001110111101011110001111111100010010000110101000001010111000111011110111010010010000101110000011001111000101010001101100000110111111110011001110101011000110010111111000101001100010001011011101010101011101010110000111100101000000110011000011001101000001010110110010000110101011111100010111011100110111101110111011001001110100100110010100111001000001010101010010100010100101101000010100010000111110101111000101110'),
            Point(FQ(16391910732431349989910402670442677728780476741314399751389577385062806845560),
                  FQ(9557600014660247419117975756584483223203707451467643504980876223495155042156)))


if __name__ == "__main__":
    unittest.main()
