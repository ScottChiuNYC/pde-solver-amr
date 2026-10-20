poetry install
poetry run pytest
poetry run python -c "from pde_solver_amr import hello; print(hello())"