#include <gtest/gtest.h>
#include <sdsl/bit_vector_il.hpp>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/bits.hpp>
#include <sdsl/bp_support.hpp>
#include <sdsl/bp_support_algorithm.hpp>
#include <sdsl/bp_support_g.hpp>
#include <sdsl/bp_support_gg.hpp>
#include <sdsl/bp_support_sada.hpp>
#include <sdsl/cereal.hpp>
#include <sdsl/coder.hpp>
#include <sdsl/coder_comma.hpp>
#include <sdsl/coder_elias_delta.hpp>
#include <sdsl/coder_elias_gamma.hpp>
#include <sdsl/coder_fibonacci.hpp>
#include <sdsl/config.hpp>
#include <sdsl/construct.hpp>
#include <sdsl/construct_bwt.hpp>
#include <sdsl/construct_config.hpp>
#include <sdsl/construct_isa.hpp>
#include <sdsl/construct_lcp.hpp>
#include <sdsl/construct_lcp_helper.hpp>
#include <sdsl/construct_sa.hpp>
#include <sdsl/construct_sa_se.hpp>
#include <sdsl/csa_alphabet_strategy.hpp>
#include <sdsl/csa_bitcompressed.hpp>
#include <sdsl/csa_sada.hpp>
#include <sdsl/csa_sampling_strategy.hpp>
#include <sdsl/csa_wt.hpp>
#include <sdsl/cst_fully.hpp>
#include <sdsl/cst_iterators.hpp>
#include <sdsl/cst_sada.hpp>
#include <sdsl/cst_sct3.hpp>
#include <sdsl/dac_vector.hpp>
#include <sdsl/divsufsort.hpp>
#include <sdsl/enc_vector.hpp>
#include <sdsl/fast_cache.hpp>
#include <sdsl/hyb_vector.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/int_vector_buffer.hpp>
#include <sdsl/int_vector_io_wrappers.hpp>
#include <sdsl/int_vector_mapper.hpp>
#include <sdsl/inv_perm_support.hpp>
#include <sdsl/io.hpp>
#include <sdsl/iterators.hpp>
#include <sdsl/k2_treap.hpp>
#include <sdsl/k2_treap_algorithm.hpp>
#include <sdsl/k2_treap_helper.hpp>
#include <sdsl/k2_tree.hpp>
#include <sdsl/k2_tree_helper.hpp>
#include <sdsl/lcp.hpp>
#include <sdsl/lcp_bitcompressed.hpp>
#include <sdsl/lcp_byte.hpp>
#include <sdsl/lcp_dac.hpp>
#include <sdsl/lcp_support_sada.hpp>
#include <sdsl/lcp_support_tree.hpp>
#include <sdsl/lcp_support_tree2.hpp>
#include <sdsl/lcp_vlc.hpp>
#include <sdsl/lcp_wt.hpp>
#include <sdsl/louds_tree.hpp>
#include <sdsl/memory_management.hpp>
#include <sdsl/memory_tracking.hpp>
#include <sdsl/nearest_neighbour_dictionary.hpp>
#include <sdsl/nn_dict_dynamic.hpp>
#include <sdsl/platform.hpp>
#include <sdsl/qsufsort.hpp>
#include <sdsl/ram_filebuf.hpp>
#include <sdsl/ram_fs.hpp>
#include <sdsl/rank_support.hpp>
#include <sdsl/rank_support_int.hpp>
#include <sdsl/rank_support_int_scan.hpp>
#include <sdsl/rank_support_int_v.hpp>
#include <sdsl/rank_support_scan.hpp>
#include <sdsl/rank_support_v.hpp>
#include <sdsl/rank_support_v5.hpp>
#include <sdsl/rmq_succinct_sada.hpp>
#include <sdsl/rmq_succinct_sct.hpp>
#include <sdsl/rmq_support.hpp>
#include <sdsl/rmq_support_sparse_table.hpp>
#include <sdsl/rrr_helper.hpp>
#include <sdsl/rrr_vector.hpp>
#include <sdsl/rrr_vector_15.hpp>
#include <sdsl/sd_vector.hpp>
#include <sdsl/sdsl_concepts.hpp>
#include <sdsl/select_support.hpp>
#include <sdsl/select_support_mcl.hpp>
#include <sdsl/select_support_scan.hpp>
#include <sdsl/sfstream.hpp>
#include <sdsl/sorted_int_stack.hpp>
#include <sdsl/sorted_multi_stack_support.hpp>
#include <sdsl/sorted_stack_support.hpp>
#include <sdsl/structure_tree.hpp>
#include <sdsl/suffix_array_algorithm.hpp>
#include <sdsl/suffix_array_helper.hpp>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/suffix_tree_algorithm.hpp>
#include <sdsl/suffix_tree_helper.hpp>
#include <sdsl/suffix_trees.hpp>
#include <sdsl/uint128_t.hpp>
#include <sdsl/uint256_t.hpp>
#include <sdsl/uintx_t.hpp>
#include <sdsl/util.hpp>
#include <sdsl/vectors.hpp>
#include <sdsl/version.hpp>
#include <sdsl/vlc_vector.hpp>
#include <sdsl/wavelet_trees.hpp>
#include <sdsl/wm_int.hpp>
#include <sdsl/wt_algorithm.hpp>
#include <sdsl/wt_ap.hpp>
#include <sdsl/wt_blcd.hpp>
#include <sdsl/wt_epr.hpp>
#include <sdsl/wt_gmr.hpp>
#include <sdsl/wt_helper.hpp>
#include <sdsl/wt_huff.hpp>
#include <sdsl/wt_hutu.hpp>
#include <sdsl/wt_int.hpp>
#include <sdsl/wt_pc.hpp>
#include <sdsl/wt_rlmn.hpp>


namespace
{
// The fixture for testing the compilation of all header files.
class CompileTest : public ::testing::Test { };

//! Test constructors
TEST_F(CompileTest, Compile) { }

}  // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
