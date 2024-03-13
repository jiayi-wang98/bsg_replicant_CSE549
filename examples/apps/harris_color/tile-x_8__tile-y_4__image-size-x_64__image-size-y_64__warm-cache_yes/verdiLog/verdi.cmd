simSetSimulator "-vcssv" -exec \
           "/local1/jwang710/bsg_bladerunner/bsg_replicant/machines/pod_X1Y1_ruche_X8Y4_hbm_one_pseudo_channel/bigblade-vcs/debug/simv" \
           -args \
           "+ntb_random_seed_automatic +c_args=main.riscv\\ harris_color +c_path=/local1/jwang710/bsg_bladerunner/bsg_replicant/examples/hb_hammerbench/apps/harris_color/tile-x_8__tile-y_4__image-size-x_64__image-size-y_64__warm-cache_yes/main.so"
debImport "-dbdir" \
          "/local1/jwang710/bsg_bladerunner/bsg_replicant/machines/pod_X1Y1_ruche_X8Y4_hbm_one_pseudo_channel/bigblade-vcs/debug/simv.daidir"
debLoadSimResult \
           /local1/jwang710/bsg_bladerunner/bsg_replicant/examples/hb_hammerbench/apps/harris_color/tile-x_8__tile-y_4__image-size-x_64__image-size-y_64__warm-cache_yes/debug.fsdb
wvCreateWindow
verdiWindowResize -win $_Verdi_1 "1454" "1169" "1755" "1313"
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
verdiSetActWin -dock widgetDock_<Inst._Tree>
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile" \
           -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore" \
           -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.tx" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.tx" \
           -delim "." -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.tx" \
           -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore" \
           -delim "." -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore" \
           -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.lsu0" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.lsu0" \
           -delim "." -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.lsu0" \
           -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_req_o" -line 42 -pos 1 -win $_nTrace1
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/vcore/lsu0/remote_req_o"
wvSetPosition -win $_nWave2 {("G1" 0)}
wvSetPosition -win $_nWave2 {("G1" 1)}
wvSetPosition -win $_nWave2 {("G1" 1)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_req_v_o" -line 43 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G2" 0)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/vcore/lsu0/remote_req_v_o"
wvSetPosition -win $_nWave2 {("G2" 0)}
wvSetPosition -win $_nWave2 {("G2" 1)}
wvSetPosition -win $_nWave2 {("G2" 1)}
wvZoomAll -win $_nWave2
verdiSetActWin -win $_nWave2
wvZoom -win $_nWave2 51134729.475685 57993337.005956
wvZoom -win $_nWave2 52124979.619509 52822655.857207
wvZoom -win $_nWave2 52309271.455873 52408285.392806
wvZoom -win $_nWave2 52328034.556306 52349884.227377
wvZoom -win $_nWave2 52332766.560535 52336458.957779
wvZoom -win $_nWave2 52333847.927073 52335001.990527
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52337363.607657 52342877.361188
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52332322.526179 52340283.318808
wvZoom -win $_nWave2 52333720.073030 52337873.530054
wvZoom -win $_nWave2 52333999.468827 52335014.834038
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52333814.970553 52334258.099587
wvZoom -win $_nWave2 52333918.573318 52333986.187757
wvSearchNext -win $_nWave2
wvSearchPrev -win $_nWave2
wvZoomAll -win $_nWave2
wvZoom -win $_nWave2 45419223.200459 56621615.499902
wvZoom -win $_nWave2 48258878.016305 49085961.943253
wvZoom -win $_nWave2 48532989.506445 48657153.869556
wvZoom -win $_nWave2 48562630.055796 48580964.416223
wvZoom -win $_nWave2 48565412.546913 48567834.066215
wvZoom -win $_nWave2 48565952.869511 48566310.435937
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 49839214.719967 55630927.745529
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 50857409.422779 54962446.382653
wvZoom -win $_nWave2 52221265.016493 52574857.207459
wvZoom -win $_nWave2 52327719.761432 52351215.222850
wvZoom -win $_nWave2 52333579.171004 52338031.551306
wvZoom -win $_nWave2 52334196.441116 52334959.810667
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52335746.663686 52336017.193339
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52336502.437868 52336779.403418
wvZoom -win $_nWave2 52336712.604500 52336744.413505
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52331571.431658 52360376.247040
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52331914.064812 52351621.379027
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.rx" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.rx" \
           -delim "." -win $_nTrace1
verdiSetActWin -dock widgetDock_<Inst._Tree>
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.rx" \
           -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "addr_i" -line 40 -pos 1 -win $_nTrace1
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_addr_o" -line 54 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G3" 0)}
wvSetPosition -win $_nWave2 {("G2" 1)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/remote_dmem_addr_o\[9:0\]"
wvSetPosition -win $_nWave2 {("G2" 1)}
wvSetPosition -win $_nWave2 {("G2" 2)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_mask_o" -line 55 -pos 1 -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_data_o" -line 56 -pos 1 -win $_nTrace1
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/remote_dmem_data_o\[31:0\]"
wvSetPosition -win $_nWave2 {("G2" 2)}
wvSetPosition -win $_nWave2 {("G2" 3)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_data_i" -line 57 -pos 1 -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_w_o" -line 53 -pos 1 -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_v_o" -line 52 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G3" 0)}
wvSetPosition -win $_nWave2 {("G2" 3)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_w_o" -line 53 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G3" 0)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/remote_dmem_w_o"
wvSetPosition -win $_nWave2 {("G3" 0)}
wvSetPosition -win $_nWave2 {("G3" 1)}
wvSetPosition -win $_nWave2 {("G3" 1)}
wvSetPosition -win $_nWave2 {("G2" 3)}
verdiSetActWin -win $_nWave2
wvMoveSelected -win $_nWave2
wvSetPosition -win $_nWave2 {("G2" 3)}
wvSetPosition -win $_nWave2 {("G2" 4)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "remote_dmem_v_o" -line 52 -pos 1 -win $_nTrace1
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
wvSetPosition -win $_nWave2 {("G3" 0)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/remote_dmem_v_o"
wvSetPosition -win $_nWave2 {("G3" 0)}
wvSetPosition -win $_nWave2 {("G3" 1)}
wvSetPosition -win $_nWave2 {("G2" 4)}
verdiSetActWin -win $_nWave2
wvMoveSelected -win $_nWave2
wvSetPosition -win $_nWave2 {("G2" 4)}
wvSetPosition -win $_nWave2 {("G2" 5)}
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 51886510.980074 53426106.507028
wvZoom -win $_nWave2 52226257.655009 52485172.407286
wvZoom -win $_nWave2 52288490.651240 52334368.901029
wvZoom -win $_nWave2 52297749.100336 52304335.395424
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
srcDeselectAll -win $_nTrace1
srcSelect -signal "returning_data_o" -line 48 -pos 1 -win $_nTrace1
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
wvSetPosition -win $_nWave2 {("G2" 2)}
wvSetPosition -win $_nWave2 {("G3" 0)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/returning_data_o\[31:0\]"
wvSetPosition -win $_nWave2 {("G3" 0)}
wvSetPosition -win $_nWave2 {("G3" 1)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "returning_data_v_o" -line 49 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G4" 0)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/returning_data_v_o"
wvSetPosition -win $_nWave2 {("G4" 0)}
wvSetPosition -win $_nWave2 {("G4" 1)}
wvSetPosition -win $_nWave2 {("G4" 1)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "addr_i" -line 40 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G5" 0)}
wvSetPosition -win $_nWave2 {("G4" 1)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/addr_i\[27:0\]"
wvSetPosition -win $_nWave2 {("G4" 1)}
wvSetPosition -win $_nWave2 {("G4" 2)}
wvSelectSignal -win $_nWave2 {( "G2" 1 )} 
verdiSetActWin -win $_nWave2
wvSelectSignal -win $_nWave2 {( "G4" 2 )} 
srcDeselectAll -win $_nTrace1
srcSelect -signal "data_i" -line 41 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G2" 1)}
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
wvSetPosition -win $_nWave2 {("G5" 0)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/rx/data_i\[31:0\]"
wvSetPosition -win $_nWave2 {("G5" 0)}
wvSetPosition -win $_nWave2 {("G5" 1)}
wvSetPosition -win $_nWave2 {("G5" 1)}
wvSelectGroup -win $_nWave2 {G5}
verdiSetActWin -win $_nWave2
wvSelectSignal -win $_nWave2 {( "G5" 1 )} 
wvSetPosition -win $_nWave2 {("G4" 2)}
wvMoveSelected -win $_nWave2
wvSetPosition -win $_nWave2 {("G4" 2)}
wvSetPosition -win $_nWave2 {("G4" 3)}
wvZoomAll -win $_nWave2
wvZoomAll -win $_nWave2
wvZoom -win $_nWave2 50144041.721313 57917130.255619
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.lsu0" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.lsu0" \
           -delim "." -win $_nTrace1
verdiSetActWin -dock widgetDock_<Inst._Tree>
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.lsu0" \
           -win $_nTrace1
wvZoom -win $_nWave2 52197310.390750 52579907.037229
verdiSetActWin -win $_nWave2
wvZoom -win $_nWave2 52329445.901952 52380605.256650
wvZoom -win $_nWave2 52332635.492563 52341322.930153
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52325348.449954 52357732.138559
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.int_sb" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.int_sb" \
           -delim "." -win $_nTrace1
verdiSetActWin -dock widgetDock_<Inst._Tree>
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.int_sb" \
           -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "scoreboard_r" -line 37 -pos 1 -win $_nTrace1
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
wvSetPosition -win $_nWave2 {("G5" 0)}
wvSetPosition -win $_nWave2 {("G6" 0)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/vcore/int_sb/scoreboard_r\[31:0\]"
wvSetPosition -win $_nWave2 {("G6" 0)}
wvSetPosition -win $_nWave2 {("G6" 1)}
wvSetPosition -win $_nWave2 {("G6" 1)}
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
verdiSetActWin -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52244628.320478 52537064.451868
wvZoomAll -win $_nWave2
wvZoomAll -win $_nWave2
wvZoomAll -win $_nWave2
wvZoom -win $_nWave2 50296455.221985 54640239.991157
wvZoom -win $_nWave2 52220691.296921 52601975.111770
wvZoom -win $_nWave2 52317654.367144 52397727.096104
wvZoom -win $_nWave2 52339659.587978 52351154.852595
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
srcDeselectAll -win $_nTrace1
srcSelect -signal "src_id_i" -line 22 -pos 1 -win $_nTrace1
srcDeselectAll -win $_nTrace1
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
srcDeselectAll -win $_nTrace1
srcSelect -signal "dest_id_i" -line 23 -pos 1 -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "op_reads_rf_i" -line 25 -pos 1 -win $_nTrace1
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/vcore/int_sb/op_reads_rf_i\[1:0\]"
wvSetPosition -win $_nWave2 {("G6" 1)}
wvSetPosition -win $_nWave2 {("G6" 2)}
srcDeselectAll -win $_nTrace1
srcSelect -signal "op_writes_rf_i" -line 26 -pos 1 -win $_nTrace1
wvSetPosition -win $_nWave2 {("G4" 2)}
wvSetPosition -win $_nWave2 {("G6" 2)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/vcore/int_sb/op_writes_rf_i"
wvSetPosition -win $_nWave2 {("G6" 2)}
wvSetPosition -win $_nWave2 {("G6" 3)}
wvSelectSignal -win $_nWave2 {( "G3" 1 )} 
verdiSetActWin -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 50935572.896023 54759507.090586
verdiSetActWin -dock widgetDock_<Inst._Tree>
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.wb_data_pipeline" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.wb_data_pipeline" \
           -delim "." -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.wb_data_pipeline" \
           -win $_nTrace1
verdiSetActWin -win $_nWave2
wvSetCursor -win $_nWave2 51964490.382334 -snap {("G6" 3)}
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.int_rf" \
           -win $_nTrace1
verdiSetActWin -dock widgetDock_<Inst._Tree>
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.int_sb" \
           -win $_nTrace1
srcSetScope \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.int_sb" \
           -delim "." -win $_nTrace1
srcHBSelect \
           "replicant_tb_top.testbench.fi1.DUT.py\[0\].podrow.px\[0\].pod.mc_y\[0\].mc_x\[0\].mc.y\[0\].x\[0\].tile.proc.h.z.vcore.int_sb" \
           -win $_nTrace1
srcDeselectAll -win $_nTrace1
srcSelect -signal "dependency_o" -line 34 -pos 1 -win $_nTrace1
verdiSetActWin -dock widgetDock_MTB_SOURCE_TAB_1
wvSetPosition -win $_nWave2 {("G7" 0)}
wvSetPosition -win $_nWave2 {("G6" 3)}
wvAddSignal -win $_nWave2 \
           "/replicant_tb_top/testbench/fi1/DUT/py\[0\]/podrow/px\[0\]/pod/mc_y\[0\]/mc_x\[0\]/mc/y\[0\]/x\[0\]/tile/proc/h/z/vcore/int_sb/dependency_o"
wvSetPosition -win $_nWave2 {("G6" 3)}
wvSetPosition -win $_nWave2 {("G6" 4)}
wvZoomOut -win $_nWave2
verdiSetActWin -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54673241.142791 55388464.273521
wvZoom -win $_nWave2 54929055.158375 54996529.038633
wvZoom -win $_nWave2 54941620.023685 54950587.020076
wvZoom -win $_nWave2 54942944.108463 54946107.199889
wvZoom -win $_nWave2 54943309.978756 54944469.865440
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 53800011.387281 54111801.107162
wvZoom -win $_nWave2 53895671.235710 53931991.285113
wvZoom -win $_nWave2 53906337.829372 53912088.255570
wvZoom -win $_nWave2 53907281.295522 53908474.780205
wvZoom -win $_nWave2 53907710.127588 53907969.580780
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvSetCursor -win $_nWave2 54532046.348078 -snap {("G2" 1)}
wvZoom -win $_nWave2 54155430.139467 54525071.973844
wvZoom -win $_nWave2 54261258.687612 54290065.885533
wvZoom -win $_nWave2 54264779.829932 54271798.482748
wvZoom -win $_nWave2 54266253.804598 54268073.242080
wvZoom -win $_nWave2 54266659.782437 54267518.007664
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54711186.304092 55587846.124558
wvZoom -win $_nWave2 54917586.114159 55060699.571314
wvZoom -win $_nWave2 54938131.524324 54983801.036083
wvZoom -win $_nWave2 54941728.138864 54950307.563140
wvZoom -win $_nWave2 54943009.070130 54945711.694348
wvZoom -win $_nWave2 54943259.600513 54943980.152499
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54943123.351750 54946215.991611
wvZoom -win $_nWave2 54943506.443306 54944072.201049
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54944058.509614 54945049.861985
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvSetCursor -win $_nWave2 54944189.849562 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 54943877.561367 -snap {("G6" 3)}
wvSetCursor -win $_nWave2 54944502.137758 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54943461.177106 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54951164.285927 -snap {("G2" 1)}
wvZoom -win $_nWave2 54948561.884298 54958138.722292
wvZoom -win $_nWave2 54951185.890763 54952513.606612
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54950969.687640 54954768.763017
wvZoom -win $_nWave2 54951814.272886 54953269.702091
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54937182.970835 54995256.864629
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54981846.035169 55019958.516739
wvZoom -win $_nWave2 54990975.522132 55001511.950589
wvZoom -win $_nWave2 54992246.116866 54996049.257572
wvZoom -win $_nWave2 54992495.707719 54993384.875152
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvSetCursor -win $_nWave2 54943316.141445 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54943876.338811 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54944623.268632 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54942942.676534 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54943782.972583 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54943222.775217 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54943969.705038 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54944436.536176 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54951065.538338 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54951812.468159 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54952372.665525 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54953212.961573 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54953773.158939 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54954333.356305 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54954520.088760 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54956574.145768 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54958534.836548 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54959841.963735 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54961149.090922 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54962362.851881 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54963763.345295 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54965257.204937 -snap {("G2" 0)}
wvSetCursor -win $_nWave2 54965257.204937 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54966004.134758 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54966751.064579 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 54967871.459311 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54969085.220270 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54970485.713684 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54971979.573326 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54973099.968058 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54974593.827700 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54975900.954886 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54983463.619324 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54993827.270591 -snap {("G2" 1)}
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 55060163.975317 55311132.395176
wvZoom -win $_nWave2 55117604.655491 55156721.964575
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54893281.809582 55133569.266859
wvSelectSignal -win $_nWave2 {( "G6" 3 )} 
wvSelectSignal -win $_nWave2 {( "G6" 2 )} 
wvSelectSignal -win $_nWave2 {( "G6" 1 )} 
wvSelectSignal -win $_nWave2 {( "G6" 2 )} 
wvSelectSignal -win $_nWave2 {( "G6" 2 )} 
wvSelectSignal -win $_nWave2 {( "G6" 2 )} 
wvSelectSignal -win $_nWave2 {( "G6" 3 )} 
wvSelectSignal -win $_nWave2 {( "G6" 2 )} 
wvSelectSignal -win $_nWave2 {( "G6" 1 )} 
wvZoom -win $_nWave2 54933296.866042 55027716.629812
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54784696.113951 55265547.544209
wvZoom -win $_nWave2 54925519.707553 54991395.170107
wvSetCursor -win $_nWave2 54942218.245638 -snap {("G6" 1)}
wvZoom -win $_nWave2 54942542.489096 54948324.830764
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 54942672.935684 54948308.228466
wvZoom -win $_nWave2 54943232.304365 54944919.656182
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvSetCursor -win $_nWave2 54944496.780067 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 54950742.334948 -snap {("G6" 0)}
wvSetCursor -win $_nWave2 54950033.619501 -snap {("G6" 2)}
wvSetCursor -win $_nWave2 54964075.044305 -snap {("G6" 1)}
wvZoom -win $_nWave2 54960841.530076 54965359.591054
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 52049210.479721 63175396.028827
wvZoom -win $_nWave2 54760020.248452 56768027.484550
wvZoom -win $_nWave2 54895095.386563 55189954.529518
wvSetCursor -win $_nWave2 54950003.528849 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 54995478.113653 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 54997655.088457 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 54992575.480580 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55081577.467170 -snap {("G6" 4)}
wvZoom -win $_nWave2 55059323.946947 55124633.191080
wvSetCursor -win $_nWave2 55075075.315090 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 55075932.532404 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55076575.445390 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55077325.510540 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55077861.271362 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55075128.891172 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55075932.532404 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55076575.445390 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55077218.358376 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55077646.967033 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55078611.336512 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55079093.521251 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55079790.010319 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55080486.499387 -snap {("G1" 1)}
wvSetCursor -win $_nWave2 55081450.868865 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 55081933.053604 -snap {("G2" 1)}
wvSetCursor -win $_nWave2 55082683.118754 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55083968.944726 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55085951.259765 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55086915.629244 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55089915.889844 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55089165.824694 -snap {("G6" 4)}
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoomOut -win $_nWave2
wvZoom -win $_nWave2 55106765.567682 55213917.731969
wvSetCursor -win $_nWave2 55140256.112875 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55135421.519737 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55125928.136847 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55134366.699416 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55140431.916262 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55140607.719649 -snap {("G6" 3)}
wvSetCursor -win $_nWave2 55154584.088904 -snap {("G6" 3)}
wvSetCursor -win $_nWave2 55158363.861721 -snap {("G6" 3)}
wvSetCursor -win $_nWave2 55159506.583736 -snap {("G6" 3)}
wvSetCursor -win $_nWave2 55165659.702276 -snap {("G6" 4)}
wvSetCursor -win $_nWave2 55164780.685341 -snap {("G6" 4)}
wvZoom -win $_nWave2 55159682.387123 55177878.037662
