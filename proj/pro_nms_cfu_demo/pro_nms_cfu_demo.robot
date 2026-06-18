*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    Reset Emulation
Test Teardown                 Test Teardown
Resource                      ${RENODEKEYWORDS}

*** Test Cases ***
Should Run PRO CFU INT8 DOT4 Demo
    Execute Command          include @${CURDIR}/TARGET.resc
    Create Terminal Tester   sysbus.uart

    Start Emulation

    Wait For Line On Uart    CFU Playground
    Wait For Prompt On Uart  main>
    Write Line To Uart       3
    Wait For Line On Uart    Project Menu
    Wait For Prompt On Uart  project>
    Write Line To Uart       0
    Wait For Line On Uart    PRO CFU DEMO - RISC-V + CFU INT8 DOT4
    Wait For Line On Uart    CPU-only dot product
    Wait For Line On Uart    result = -?\\d+                                      treatAsRegex=true
    Wait For Line On Uart    cycles = \\d+                                        treatAsRegex=true
    Wait For Line On Uart    CFU custom instruction dot product
    Wait For Line On Uart    result = -?\\d+                                      treatAsRegex=true
    Wait For Line On Uart    cycles = \\d+                                        treatAsRegex=true
    Wait For Line On Uart    VERIFY = OK
    Wait For Line On Uart    SPEEDUP = \\d+\\.\\d+ x                              treatAsRegex=true
    Wait For Line On Uart    CFU Playground / LiteX / VexRiscV / Renode
