import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from capture import parse_row
from analyse import summarise
class ToolsTest(unittest.TestCase):
    def test_console_headers_and_corruption(self):
        self.assertIsNone(parse_row('# boot'))
        self.assertIsNone(parse_row('uptime_ms,sequence'))
        with self.assertRaises(ValueError): parse_row('100,1,1,1000,1000,1000,500,50,0,VALID,4,READY,0')
    def test_summary_ignores_heartbeat_and_fault_depth(self):
        row=parse_row('100,1,1,1000,1000,1000,500,50,0,VALID,1,READY,0')
        row['reference_depth_mm']='490'
        fault=dict(row,validity='INVALID',permission='0',depth_mm='0')
        heartbeat=dict(row,sample_event='0')
        s=summarise([row,heartbeat,fault])
        self.assertEqual(s['mae_mm'],10)
        self.assertEqual(s['sample_events'],2)
        self.assertEqual(s['permission_transitions'],1)
if __name__=='__main__':unittest.main()
