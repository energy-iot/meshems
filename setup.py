"""
Setup script for EMS-Dev Python Gateway
"""

from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("requirements.txt", "r", encoding="utf-8") as fh:
    requirements = [line.strip() for line in fh if line.strip() and not line.startswith("#")]

setup(
    name="ems-dev-python",
    version="1.0.0",
    author="Energy IoT Open Source",
    author_email="info@energy-iot.org",
    description="Python implementation of EMS-Dev for Sol-Ark inverters with SunSpec support",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/energy-iot/ems-dev-python",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Topic :: System :: Hardware",
        "Topic :: System :: Monitoring",
        "Topic :: Scientific/Engineering",
    ],
    python_requires=">=3.8",
    install_requires=requirements,
    entry_points={
        "console_scripts": [
            "ems-dev=ems.main:main",
        ],
    },
    include_package_data=True,
    package_data={
        "ems": ["*.yaml", "*.json"],
    },
)