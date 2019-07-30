pragma solidity ^0.5.0;

import "truffle/Assert.sol";
import "../contracts/MiMC_permutation.sol";
import "../contracts/MiMC_hash.sol";


contract TestMiMC
{
	/**
	* Encrypt a single message with one key
	*
	* This is *NOT* a demonstrative way to securely encrypt data
	*/
	function testEncipher_solidity () public
	{
		uint256 x = 3703141493535563179657531719960160174296085208671919316200479060314459804651;
		uint256 k = 134551314051432487569247388144051420116740427803855572138106146683954151557;
		uint256 expected = 11437467823393790387399137249441941313717686441929791910070352316474327319704;
		uint256 result = MiMC_permutation.MiMCpe7(x, k);
		Assert.equal(result, expected, "Unexpected result");
	}

	function testHash () public
	{
		uint256[] memory m = new uint256[](2);
        m[0] = 3703141493535563179657531719960160174296085208671919316200479060314459804651;
        m[1] = 134551314051432487569247388144051420116740427803855572138106146683954151557;
		uint256 IV = 918403109389145570117360101535982733651217667914747213867238065296420114726;
		uint256 expected = 15683951496311901749339509118960676303290224812129752890706581988986633412003;
		uint256 result = MiMC_hash.MiMCpe7_mp(m, IV);
		Assert.equal(result, expected, "Unexpected result");
	}
}
