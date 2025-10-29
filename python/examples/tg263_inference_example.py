"""
TG-263 Structure Name Translation - Inference Examples

Demonstrates how to use the trained TG-263 model to translate
clinical structure names to standardized nomenclature.

Copyright (C) 2nd Messenger Systems
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.tg263_model import TG263Translator


def example_basic_translation():
    """Basic example: translate a single structure name"""
    print("=" * 70)
    print("Example 1: Basic Translation")
    print("=" * 70)

    # Load trained model
    model_path = 'tg263_model.pth'
    translator = TG263Translator.load(model_path, device='cpu')

    # Translate a structure name
    clinical_name = "left parotid gland"
    predictions = translator.predict(clinical_name, top_k=3)

    print(f"\nInput: '{clinical_name}'")
    print("\nTop predictions:")
    for i, (standard_name, confidence) in enumerate(predictions, 1):
        print(f"  {i}. {standard_name:20s} (confidence: {confidence:.3f})")


def example_batch_translation():
    """Example: translate multiple structure names at once"""
    print("\n" + "=" * 70)
    print("Example 2: Batch Translation")
    print("=" * 70)

    # Load model
    translator = TG263Translator.load('tg263_model.pth', device='cpu')

    # List of clinical structure names
    clinical_names = [
        "Left Parotid",
        "RIGHT LUNG",
        "Spinal Cord",
        "Lt Femoral Head",
        "Rectum",
        "PTV Primary",
        "Brain Stem",
        "L Kidney",
        "Heart",
    ]

    # Translate batch
    results = translator.predict_batch(clinical_names, top_k=1)

    print("\nBatch translation results:")
    print(f"{'Clinical Name':<25} | {'Standard Name':<20} | Confidence")
    print("-" * 70)

    for clinical_name, predictions in zip(clinical_names, results):
        if predictions:
            standard_name, confidence = predictions[0]
            print(f"{clinical_name:<25} | {standard_name:<20} | {confidence:.3f}")


def example_uncertainty_detection():
    """Example: detect uncertain translations"""
    print("\n" + "=" * 70)
    print("Example 3: Uncertainty Detection")
    print("=" * 70)

    translator = TG263Translator.load('tg263_model.pth', device='cpu')

    # Test names with varying clarity
    test_cases = [
        "Parotid_L",  # Clear
        "Left Parotid",  # Clear
        "Lt Par",  # Ambiguous abbreviation
        "Xyz123",  # Nonsense
        "Submandibular Gland Left",  # Clear but verbose
    ]

    confidence_threshold = 0.7
    print(f"\nClassifying confidence (threshold: {confidence_threshold}):\n")

    for name in test_cases:
        predictions = translator.predict(name, top_k=3)

        if not predictions:
            status = "FAILED"
            print(f"'{name}': {status} - No predictions")
            continue

        top_pred, top_conf = predictions[0]

        if top_conf >= confidence_threshold:
            status = "HIGH"
        elif top_conf >= 0.5:
            status = "MEDIUM"
        else:
            status = "LOW"

        print(f"'{name}': {status} confidence")
        print(f"  → {top_pred} ({top_conf:.3f})")

        if top_conf < confidence_threshold:
            print(f"  Alternative predictions:")
            for pred_name, conf in predictions[1:]:
                print(f"    - {pred_name} ({conf:.3f})")
        print()


def example_rtss_structure_mapping():
    """Example: map DICOM RTSS structure names to TG-263"""
    print("\n" + "=" * 70)
    print("Example 4: DICOM RTSS Structure Mapping")
    print("=" * 70)

    translator = TG263Translator.load('tg263_model.pth', device='cpu')

    # Simulated RTSS structure set (like from pydicom)
    rtss_structures = {
        1: "zPTV_7000",
        2: "zPTV_5600",
        3: "Parotid_L",
        4: "Parotid_R",
        5: "SpinalCord_PRV",
        6: "Brainstem",
        7: "Oral Cavity",
        8: "Mandible",
        9: "BODY",
    }

    print("\nMapping RTSS structures to TG-263 standard:")
    print(f"{'ROI #':<8} | {'Clinical Name':<25} | {'TG-263 Standard':<20} | Conf")
    print("-" * 80)

    tg263_mapping = {}
    for roi_number, clinical_name in rtss_structures.items():
        predictions = translator.predict(clinical_name, top_k=1)

        if predictions:
            standard_name, confidence = predictions[0]
            tg263_mapping[roi_number] = {
                'clinical_name': clinical_name,
                'standard_name': standard_name,
                'confidence': confidence
            }

            print(f"{roi_number:<8} | {clinical_name:<25} | {standard_name:<20} | {confidence:.3f}")
        else:
            print(f"{roi_number:<8} | {clinical_name:<25} | {'UNKNOWN':<20} | 0.000")

    return tg263_mapping


def example_integration_with_structure_class():
    """Example: integrate with Brimstone Structure class"""
    print("\n" + "=" * 70)
    print("Example 5: Integration with Brimstone Structure Class")
    print("=" * 70)

    try:
        from pybrimstone import Structure
        has_brimstone = True
    except ImportError:
        print("\nNote: Brimstone not installed. Showing code example only.")
        has_brimstone = False

    translator = TG263Translator.load('tg263_model.pth', device='cpu')

    # Code example
    print("\nCode example for integration:")
    print("""
    from pybrimstone import Structure
    from pybrimstone.tg263_model import TG263Translator

    # Load TG-263 translator
    translator = TG263Translator.load('tg263_model.pth')

    # Create structure with clinical name
    structure = Structure()
    clinical_name = "Left Parotid Gland"

    # Translate to TG-263 standard
    predictions = translator.predict(clinical_name, top_k=1)
    if predictions:
        standard_name, confidence = predictions[0]

        if confidence >= 0.7:
            # High confidence - use standard name
            structure.set_name(standard_name)
            print(f"Standardized: {clinical_name} → {standard_name}")
        else:
            # Low confidence - keep original and log
            structure.set_name(clinical_name)
            print(f"Warning: Low confidence ({confidence:.3f}) for {clinical_name}")
            print(f"Suggested: {standard_name}")
    """)

    if has_brimstone:
        print("\nLive demonstration:")
        # Create structures and translate names
        test_names = ["Left Lung", "Right Kidney", "Spinal Cord"]

        for name in test_names:
            structure = Structure()
            predictions = translator.predict(name, top_k=1)

            if predictions:
                standard_name, confidence = predictions[0]
                print(f"  {name:<20} → {standard_name:<20} (conf: {confidence:.3f})")


def example_statistics_and_coverage():
    """Example: analyze model coverage and statistics"""
    print("\n" + "=" * 70)
    print("Example 6: Model Statistics and Coverage")
    print("=" * 70)

    translator = TG263Translator.load('tg263_model.pth', device='cpu')

    print(f"\nModel Information:")
    print(f"  Number of standard names: {len(translator.label_to_name)}")
    print(f"  Vocabulary size: {len(translator.char_to_idx)}")
    print(f"  Maximum name length: {translator.max_length}")

    print(f"\nSupported TG-263 standard names:")

    # Group by anatomical category (simple heuristic)
    categories = {
        'Brain/CNS': ['Brain', 'Brainstem', 'Chiasm', 'Cochlea', 'Lens', 'OpticNrv'],
        'Head & Neck': ['Parotid', 'Submand', 'Larynx', 'Mandible', 'Esophagus'],
        'Thorax': ['Heart', 'Lung', 'Trachea'],
        'Abdomen': ['Liver', 'Stomach', 'Kidney', 'Spleen', 'Bowel'],
        'Pelvis': ['Bladder', 'Rectum', 'FemoralHead', 'Prostate', 'Uterus'],
        'Spine': ['SpinalCord', 'SpinalCanal', 'Cauda'],
        'Targets': ['GTV', 'CTV', 'PTV', 'ITV'],
        'Other': ['Body', 'BoneMarrow'],
    }

    for category, keywords in categories.items():
        matching_names = [
            name for name in translator.label_to_name.values()
            if any(kw in name for kw in keywords)
        ]

        if matching_names:
            print(f"\n  {category}:")
            for name in sorted(matching_names):
                print(f"    - {name}")


def main():
    """Run all examples"""
    print("\nTG-263 Structure Name Translator - Examples")
    print("Copyright (C) 2nd Messenger Systems\n")

    try:
        example_basic_translation()
        example_batch_translation()
        example_uncertainty_detection()
        example_rtss_structure_mapping()
        example_integration_with_structure_class()
        example_statistics_and_coverage()

        print("\n" + "=" * 70)
        print("All examples completed successfully!")
        print("=" * 70)

    except FileNotFoundError as e:
        print(f"\nError: Model file not found: {e}")
        print("\nPlease train the model first:")
        print("  python -m pybrimstone.tg263_training")
    except Exception as e:
        print(f"\nError: {e}")
        import traceback
        traceback.print_exc()


if __name__ == '__main__':
    main()
