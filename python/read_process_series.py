"""_summary_"""

import itk
import itk.ITKTransformPython
from itk.itkImagePython import itkImageF3
import itk.itkLinearInterpolateImageFunctionPython
import itk.itkResampleImageFilterPython
import numpy as np


def _conform_to(from_img: itkImageF3, to_img: itkImageF3):
    if (
        to_img.GetLargestPossibleRegion() != from_img.GetLargestPossibleRegion()
        and to_img.GetBufferedRegion() != from_img.GetBufferedRegion()
    ):
        to_img.SetLargestPossibleRegion(from_img.GetLargestPossibleRegion())
        to_img.SetBufferedRegion(from_img.GetBufferedRegion())
        to_img.Allocate()

    to_img.SetRequestedRegion(from_img.GetRequestedRegion())

    to_img.SetOrigin(from_img.GetOrigin())
    to_img.SetSpacing(from_img.GetSpacing())
    to_img.SetDirection(from_img.GetDirection())


def ct_to_md_values(ct_img: itkImageF3) -> itkImageF3:
    """_summary_

    Args:
        img (_type_): _description_

    Returns:
        _type_: _description_
    """
    # Convert ITK image to NumPy array
    ct_array = itk.array_from_image(ct_img)
    ct_array = ct_array.copy()  # Clone the NumPy array

    # Create output array with same shape as input
    md_array = np.zeros_like(ct_array, dtype=np.float32)

    # Condition 1: values < 0
    mask_air = ct_array < 0
    md_array[mask_air] = 0.0 + 1.0 * (ct_array[mask_air] + 1024.0) / 1024.0

    # Condition 2: 0 <= values < 1024
    mask_water = (ct_array >= 0) & (ct_array < 1024)
    # Note: this simplifies to just 1.0
    md_array[mask_water] = 1.0 + 0.0 * ct_array[mask_water] / 1024.0

    # Condition 3: values >= 1024
    mask_max = ct_array >= 1024
    md_array[mask_max] = 1.0  # Using 1.0 instead of 1.5 as per the commented value

    # Convert NumPy array back to ITK image
    md_img = itk.image_from_array(md_array)
    _conform_to(ct_img, md_img)
    return md_img


def rotate_density_for_beam(beam: dict, md_img: itkImageF3) -> itkImageF3:
    gantry_angle_deg = beam["gantry_angle_deg"]

    rotate_xform = itk.ITKTransformPython.itkEuler3DTransformD()
    rotate_xform.SetRotation(0.0, 0.0, gantry_angle_deg)
    rotate_xform.SetCenter((0.0, 0.0, 0.0))

    resampler = itk.itkResampleImageFilterPython.itkResampleImageFilterVIF3VIF3()
    resampler.SetInput(md_img)
    resampler.SetTransform(rotate_xform)

    interpolator = (
        itk.itkLinearInterpolateImageFunctionPython.itkLinearInterpolateImageFunctionICVF33D()
    )
    resampler.SetInterpolator(interpolator)

    resampler.Update()
    rotated_md_img = resampler.GetOutput()
    return rotated_md_img


def terma_from_density(beam: dict, md_img: itkImageF3) -> itkImageF3:
    terma_img = itkImageF3()
    _conform_to(md_img, terma_img)
    terma_img.FillBuffer(0.0)  # zero terma_img

    # rotate to beam
    rotate_md_img = rotate_density_for_beam(beam, md_img)

    # now perform terma calculation
    # based on rays per voxel -- in voxel coordinates
    delta_ray = 1.0 / rays_per_voxel

    # initial fluence per ray -- set so incident fluence is constant
    pixel_spacing = md_img.GetSpacing()
    fluence_0 = pixel_spacing[0] * pixel_spacing[1] * delta_ray * delta_ray

    fluence_surface_integral = 0.0

    v_x = v_min_vxl
    while v_x[0] < v_max_vxl[0]:
        v_y = v_x
        while v_y[1] < v_max_vxl[1]:
            trace_ray_terma(v_y, fluence_0)
            v_y[1] += delta_ray

        v_x[0] += delta_ray

    return terma_img


def trace_ray_terma(v_ray, fluence0):
    pass


if __name__ == "__main__":
    ct_img = itk.imread("E:/datasets/miccai_hn_sharpe/0522c0081/img.nrrd")

    # convert to mass density
    md_img = ct_to_md_values(ct_img)

    print(ct_img[0, 0, 0])
    print(md_img[0, 0, 0])
