"""
TG-263 Integration with pymedphys

This module shows how to integrate the TG-263 translator with pymedphys
for standardizing structure names in DICOM RTSS files.

Copyright (C) 2nd Messenger Systems
"""

import sys
from pathlib import Path
from typing import List, Dict, Optional, Tuple

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.tg263_model import TG263Translator


class PymedphysTG263Mapper:
    """
    Integration class for mapping pymedphys structure names to TG-263 standard

    This class provides utilities for:
    - Loading DICOM RTSS files via pymedphys
    - Translating structure names to TG-263 standard
    - Generating standardized structure reports
    - Exporting TG-263-compliant RTSS files
    """

    def __init__(
        self,
        model_path: str = 'tg263_model.pth',
        confidence_threshold: float = 0.7,
        device: str = 'cpu'
    ):
        """
        Initialize the mapper

        Args:
            model_path: Path to trained TG-263 model
            confidence_threshold: Minimum confidence for automatic mapping
            device: PyTorch device ('cpu' or 'cuda')
        """
        self.translator = TG263Translator.load(model_path, device=device)
        self.confidence_threshold = confidence_threshold

    def map_rtss_structures(
        self,
        rtss_path: str,
        auto_rename: bool = False
    ) -> Dict:
        """
        Map structure names in a DICOM RTSS file to TG-263 standard

        Args:
            rtss_path: Path to DICOM RTSS file
            auto_rename: Automatically rename structures with high confidence

        Returns:
            Dictionary containing mapping results
        """
        try:
            import pydicom
        except ImportError:
            print("Error: pydicom not installed. Install with: pip install pydicom")
            return {}

        # Load RTSS file
        rtss = pydicom.dcmread(rtss_path)

        # Extract structure set ROI sequence
        if not hasattr(rtss, 'StructureSetROISequence'):
            print("Error: No StructureSetROISequence found in RTSS file")
            return {}

        # Process each structure
        mapping_results = {
            'file': rtss_path,
            'total_structures': len(rtss.StructureSetROISequence),
            'high_confidence': 0,
            'medium_confidence': 0,
            'low_confidence': 0,
            'structures': []
        }

        print(f"\nProcessing {len(rtss.StructureSetROISequence)} structures from:")
        print(f"  {rtss_path}\n")

        for roi in rtss.StructureSetROISequence:
            roi_number = roi.ROINumber
            original_name = roi.ROIName

            # Translate to TG-263
            predictions = self.translator.predict(original_name, top_k=3)

            if predictions:
                standard_name, confidence = predictions[0]

                # Classify confidence
                if confidence >= self.confidence_threshold:
                    conf_class = 'high'
                    mapping_results['high_confidence'] += 1
                elif confidence >= 0.5:
                    conf_class = 'medium'
                    mapping_results['medium_confidence'] += 1
                else:
                    conf_class = 'low'
                    mapping_results['low_confidence'] += 1

                structure_info = {
                    'roi_number': roi_number,
                    'original_name': original_name,
                    'tg263_name': standard_name,
                    'confidence': confidence,
                    'confidence_class': conf_class,
                    'alternatives': predictions[1:],
                    'renamed': False
                }

                # Auto-rename if requested and high confidence
                if auto_rename and confidence >= self.confidence_threshold:
                    roi.ROIName = standard_name
                    structure_info['renamed'] = True

                mapping_results['structures'].append(structure_info)

                # Print mapping
                status = "✓" if conf_class == 'high' else ("?" if conf_class == 'medium' else "✗")
                print(f"{status} ROI {roi_number:3d}: {original_name:<30s} → {standard_name:<20s} ({confidence:.3f})")

        # Save modified RTSS if auto-rename was used
        if auto_rename:
            output_path = Path(rtss_path).with_suffix('.tg263.dcm')
            rtss.save_as(output_path)
            mapping_results['output_file'] = str(output_path)
            print(f"\nSaved TG-263-renamed RTSS to: {output_path}")

        return mapping_results

    def generate_mapping_report(
        self,
        mapping_results: Dict,
        output_path: Optional[str] = None
    ) -> str:
        """
        Generate a human-readable mapping report

        Args:
            mapping_results: Results from map_rtss_structures
            output_path: Optional path to save report

        Returns:
            Report text
        """
        report = []
        report.append("=" * 80)
        report.append("TG-263 Structure Name Mapping Report")
        report.append("=" * 80)
        report.append(f"\nSource File: {mapping_results['file']}")
        report.append(f"Total Structures: {mapping_results['total_structures']}")
        report.append(f"\nConfidence Summary:")
        report.append(f"  High (≥{self.confidence_threshold:.0%}):   {mapping_results['high_confidence']}")
        report.append(f"  Medium (≥50%): {mapping_results['medium_confidence']}")
        report.append(f"  Low (<50%):    {mapping_results['low_confidence']}")

        report.append(f"\n{'-' * 80}")
        report.append(f"{'ROI':<5} | {'Original Name':<30} | {'TG-263 Name':<20} | Confidence")
        report.append(f"{'-' * 80}")

        for struct in mapping_results['structures']:
            roi_num = struct['roi_number']
            orig = struct['original_name']
            tg263 = struct['tg263_name']
            conf = struct['confidence']
            renamed = " (renamed)" if struct.get('renamed') else ""

            report.append(f"{roi_num:<5} | {orig:<30} | {tg263:<20} | {conf:.3f}{renamed}")

        # Add low confidence warnings
        low_conf_structs = [
            s for s in mapping_results['structures']
            if s['confidence'] < self.confidence_threshold
        ]

        if low_conf_structs:
            report.append(f"\n{'-' * 80}")
            report.append(f"Low Confidence Mappings (Manual Review Recommended):")
            report.append(f"{'-' * 80}")

            for struct in low_conf_structs:
                report.append(f"\nROI {struct['roi_number']}: {struct['original_name']}")
                report.append(f"  Primary: {struct['tg263_name']} ({struct['confidence']:.3f})")

                if struct['alternatives']:
                    report.append(f"  Alternatives:")
                    for alt_name, alt_conf in struct['alternatives']:
                        report.append(f"    - {alt_name} ({alt_conf:.3f})")

        report.append(f"\n{'=' * 80}")
        report.append(f"Report generated by pybrimstone TG-263 Translator")
        report.append(f"{'=' * 80}\n")

        report_text = '\n'.join(report)

        if output_path:
            with open(output_path, 'w') as f:
                f.write(report_text)
            print(f"\nReport saved to: {output_path}")

        return report_text


def example_pymedphys_integration():
    """
    Example showing integration with pymedphys workflow
    """
    print("TG-263 + pymedphys Integration Example")
    print("=" * 80)

    # Create mapper
    mapper = PymedphysTG263Mapper(
        model_path='tg263_model.pth',
        confidence_threshold=0.7
    )

    # Example RTSS file path (user would provide actual path)
    rtss_path = '/path/to/rtss.dcm'

    print("\nExample 1: Analyze structure names (no modification)")
    print("-" * 80)
    print("""
    # Load and analyze RTSS structure names
    mapper = PymedphysTG263Mapper('tg263_model.pth', confidence_threshold=0.7)
    results = mapper.map_rtss_structures('patient_rtss.dcm', auto_rename=False)

    # Generate report
    report = mapper.generate_mapping_report(results, 'tg263_report.txt')
    print(report)
    """)

    print("\nExample 2: Auto-rename high-confidence structures")
    print("-" * 80)
    print("""
    # Automatically rename structures with confidence ≥ 70%
    mapper = PymedphysTG263Mapper('tg263_model.pth', confidence_threshold=0.7)
    results = mapper.map_rtss_structures('patient_rtss.dcm', auto_rename=True)
    # Creates: patient_rtss.tg263.dcm with standardized names

    print(f"Renamed {results['high_confidence']} structures")
    print(f"Review needed for {results['low_confidence']} structures")
    """)

    print("\nExample 3: Integration with pymedphys DVH analysis")
    print("-" * 80)
    print("""
    import pymedphys
    from pybrimstone.tg263_model import TG263Translator

    # Load translator
    translator = TG263Translator.load('tg263_model.pth')

    # Load dose and structure data
    dose = pymedphys.dicom.read_dicom_dose('dose.dcm')
    rtss = pymedphys.dicom.read_dicom_rtss('rtss.dcm')

    # Calculate DVH with standardized names
    dvh_data = {}
    for structure in rtss.get_structures():
        original_name = structure.name

        # Translate to TG-263
        predictions = translator.predict(original_name, top_k=1)
        if predictions:
            standard_name, confidence = predictions[0]

            # Calculate DVH
            dvh = pymedphys.dvh.calculate_dvh(dose, structure)

            # Store with standardized name
            dvh_data[standard_name] = {
                'original_name': original_name,
                'dvh': dvh,
                'confidence': confidence
            }

    # Now you can compare DVHs across patients using standard names
    """)

    print("\nExample 4: Quality assurance workflow")
    print("-" * 80)
    print("""
    # QA workflow: check for non-standard names in a batch of plans
    import glob
    from collections import Counter

    mapper = PymedphysTG263Mapper('tg263_model.pth', confidence_threshold=0.7)

    non_standard_names = Counter()
    low_confidence_mappings = []

    for rtss_file in glob.glob('/plans/**/*.dcm', recursive=True):
        results = mapper.map_rtss_structures(rtss_file)

        for struct in results['structures']:
            if struct['confidence'] < 0.7:
                non_standard_names[struct['original_name']] += 1
                low_confidence_mappings.append({
                    'file': rtss_file,
                    'structure': struct
                })

    # Report most common non-standard names
    print("Most common non-standard structure names:")
    for name, count in non_standard_names.most_common(20):
        print(f"  {name}: {count} occurrences")

    # These might need custom aliases added to the model
    """)


def example_contributing_to_pymedphys():
    """Show how this could be contributed to pymedphys"""
    print("\n" + "=" * 80)
    print("Contributing TG-263 Translator to pymedphys")
    print("=" * 80)

    print("""
    The TG-263 translator could be integrated into pymedphys as a new module:

    Proposed pymedphys API:
    -----------------------

    # Option 1: Direct integration
    import pymedphys

    # Translate structure names
    translator = pymedphys.tg263.load_translator()
    standard_name = translator.translate('left parotid')
    print(standard_name)  # 'Parotid_L'

    # Standardize RTSS file
    rtss = pymedphys.dicom.read_dicom_rtss('patient.dcm')
    standardized_rtss = pymedphys.tg263.standardize_rtss(rtss, confidence_threshold=0.7)
    standardized_rtss.save('patient_tg263.dcm')

    # Option 2: Plugin/extension pattern
    from pymedphys.experimental import tg263

    with tg263.Translator() as translator:
        results = translator.batch_translate([
            'Left Parotid',
            'Right Lung',
            'Spinal Cord'
        ])


    Integration Steps:
    ------------------

    1. Package the model:
       - Model weights as a downloadable asset
       - Lazy loading on first use
       - Fallback to rule-based system if model unavailable

    2. Add to pymedphys.dicom module:
       - New tg263.py module
       - Integration with existing RTSS readers/writers
       - Automatic standardization option

    3. Documentation:
       - Tutorial notebook showing usage
       - API documentation
       - Examples for common workflows

    4. Testing:
       - Unit tests with diverse structure names
       - Integration tests with real DICOM files
       - Performance benchmarks

    5. CLI tool:
       pymedphys tg263 standardize input.dcm -o output.dcm
       pymedphys tg263 translate "Left Parotid"
       pymedphys tg263 report input.dcm


    Benefits for pymedphys users:
    ------------------------------
    - Standardized structure names across institutions
    - Easier multi-institutional studies
    - Automatic QA for structure nomenclature
    - Integration with existing pymedphys DVH/dose analysis tools
    - Consistent reporting and documentation
    """)


def main():
    """Run integration examples"""
    print("\npybrimstone TG-263 + pymedphys Integration")
    print("Copyright (C) 2nd Messenger Systems\n")

    example_pymedphys_integration()
    example_contributing_to_pymedphys()

    print("\n" + "=" * 80)
    print("Integration examples complete!")
    print("=" * 80)
    print("\nNext steps:")
    print("  1. Train the model: python -m pybrimstone.tg263_training")
    print("  2. Test inference: python examples/tg263_inference_example.py")
    print("  3. Try with your RTSS files using the PymedphysTG263Mapper class")
    print("  4. Consider contributing to pymedphys!")
    print("=" * 80 + "\n")


if __name__ == '__main__':
    main()
